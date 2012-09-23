#include <iostream>

#include "common/Rectangle.h"
#include "common/SDL_utils.h"

#include "panicfire/ui/Driver.h"

using namespace Common;
using namespace PanicFire;
using namespace PanicFire::Common;

namespace PanicFire {

namespace UI {

Driver::Driver(Common::WorldInterface& w)
	: ::Common::Driver(800, 600, "Panic Fire"),
	mWorld(w),
	mCameraZoom(10.0f),
	mCameraZoomVelocity(0.0f),
	mMyTeamID(TeamID(1)),
	mMoving(false),
	mAI(w),
	mGameOver(false)
{
	mCamera.x = mCameraZoom;
	mCamera.y = mCameraZoom;
	mGrassTexture = new Texture("share/grass.png", 0, 0);
	mVegetationTexture = new Texture("share/vegetation.png", 0, 0);

	SDLSurface soldsurf("share/soldier.png");
	for(int i = 0; i < 2; i++) {
		SDLSurface surf(soldsurf);
		surf.mapPixelColor( [&] (const Color& c) { return mapSideColor(i == 0, c); } );
		mSoldierTextures[i] = new Texture(surf, 0, 0);
	}
}

Driver::~Driver()
{
	delete mGrassTexture;
	delete mVegetationTexture;
	for(auto t : mSoldierTextures)
		delete t;
}

bool Driver::init()
{
	SDL_utils::setupOrthoScreen(getScreenWidth(), getScreenHeight());
	if(!mData.sync(mWorld))
		return false;

	mAStar.setMapData(mData.getMapData());
	tryCenterCamera();

	return true;
}

bool Driver::prerenderUpdate(float frameTime)
{
	mCamera += mCameraVelocity * frameTime;
	mCameraZoom += mCameraZoomVelocity * frameTime;
	mTileWidth = std::max(getScreenWidth(), getScreenHeight()) / (mCameraZoom * 2.0f);

	handleEvents();
	sendInput();

	return false;
}

void Driver::sendInput()
{
	if(mGameOver)
		return;

	if(!mPathLine.empty() && !mMoving) {
		auto sd = mData.getCurrentSoldier();
		assert(sd);
		for(auto pit = mPathLine.begin(); pit != mPathLine.end(); ) {
			if(sd->position == *pit) {
				pit = mPathLine.erase(pit);
			} else {
				MovementInput i(mData.getCurrentSoldierID(), *pit);
				if(mData.movementAllowed(i)) {
					bool succ = mWorld.input(i);
					assert(succ);
					if(succ) {
						mMoving = true;
						mMovementPosition = *pit;
						mCommandedSoldierID = sd->id;
					}
				}
				break;
			}
		}
	}
}

void Driver::sendEndOfTurn()
{
	if(mData.getCurrentTeamID() != mMyTeamID)
		return;

	if(mGameOver)
		return;

	bool succ = mWorld.input(FinishTurnInput());
	assert(succ);
	mPathLine.clear();
	mMoving = false;
}

void Driver::shootAt(const Common::Position& tgtpos)
{
	if(mData.getCurrentTeamID() != mMyTeamID)
		return;

	if(mGameOver)
		return;

	bool succ = mWorld.input(ShotInput(mData.getCurrentSoldierID(), tgtpos));
	if(!succ) {
		/* TODO: display this on GUI instead. */
		std::cout << "Unable to shoot.\n";
	}
}

void Driver::handleEvents()
{
	while(1) {
		auto ev = mWorld.pollEvents(mMyTeamID);
		bool empty = boost::apply_visitor(mData, ev);
		if(empty)
			break;
		boost::apply_visitor(*this, ev);
	}

	if(mData.getCurrentTeamID() != mMyTeamID) {
		mAI.act();
	}
}

void Driver::operator()(const Common::InputEvent& ev)
{
	boost::apply_visitor(*this, ev.input);
}

void Driver::operator()(const Common::SightingEvent& ev)
{
	std::cout << "Sighting!\n";
}

void Driver::operator()(const Common::SoldierWoundedEvent& ev)
{
	/* TODO: display in GUI */
	std::cout << "Soldier got shot at!\n";
}

void Driver::operator()(const Common::GameWonEvent& ev)
{
	/* TODO: display in GUI */
	std::cout << "Game won by team " << ev.winner.id << "\n";
	mGameOver = true;
}

void Driver::operator()(const Common::EmptyEvent& ev)
{
	assert(0);
}

void Driver::operator()(const Common::MovementInput& ev)
{
	tryCenterCamera();
	if(mMoving &&
			mData.teamIDFromSoldierID(ev.mover) == mMyTeamID &&
			ev.mover == mCommandedSoldierID && ev.to == mMovementPosition) {
		mMoving = false;
	}
}

void Driver::operator()(const Common::ShotInput& ev)
{
}

void Driver::operator()(const Common::FinishTurnInput& ev)
{
	updateCurrentSoldier();
	tryCenterCamera();
}

void Driver::getVisibleMapCoordinates(Position& tl, Position& br) const
{
	unsigned int miny = std::max(0.0f, mCamera.y - mCameraZoom);
	unsigned int minx = std::max(0.0f, mCamera.x - mCameraZoom);
	const MapData* map = mData.getMapData();
	unsigned int maxy = std::min(map->getHeight(), (unsigned int)(mCamera.y + mCameraZoom + 1));
	unsigned int maxx = std::min(map->getWidth(),  (unsigned int)(mCamera.x + mCameraZoom + 1));
	tl.x = minx;
	tl.y = miny;
	br.x = maxx;
	br.y = maxy;
}

void Driver::drawFrame()
{
	Position tl, br;
	getVisibleMapCoordinates(tl, br);
	unsigned int minx, miny, maxx, maxy;
	minx = tl.x; miny = tl.y; maxx = br.x; maxy = br.y;

	const MapData* map = mData.getMapData();

	for(unsigned int j = miny; j < maxy; j++) {
		for(unsigned int i = minx; i < maxx; i++) {
			auto fr = map->getPoint(i, j);
			drawGrassTile(i, j, fr.grasslevel);
		}
	}

	for(unsigned int j = miny; j < maxy; j++) {
		for(unsigned int i = minx; i < maxx; i++) {
			auto fr = map->getPoint(i, j);
			drawVegetationTile(i, j, fr.vegetationlevel);
		}
	}

	for(unsigned int t = 1; t <= MAX_NUM_TEAMS; t++) {
		TeamData* td = mData.getTeam(TeamID(t));
		assert(td);
		if(td) {
			for(unsigned int s = 0; s < MAX_TEAM_SOLDIERS; s++) {
				SoldierID sid = td->soldiers[s];
				if(sid.id) {
					SoldierData* sd = mData.getSoldier(sid);
					assert(sd);
					if(sd) {
						const Position& p = sd->position;
						if(sd->health.value > 0 &&
								p.x >= minx && p.x < maxx &&
								p.y >= miny && p.y < maxy) {
							drawSoldierTile(p.x, p.y, sd->direction, td->id);
						}
					}
				}
			}
		}
	}

	for(unsigned int j = miny; j < maxy; j++) {
		for(unsigned int i = minx; i < maxx; i++) {
			auto fr = map->getPoint(i, j);
			drawVegetationTile(i, j, fr.vegetationlevel);
		}
	}

	{
		float htw = mTileWidth * 0.5f;
		Position opos(UINT_MAX, 0);
		for(auto& p : mPathLine) {
			if(opos.x != UINT_MAX) {
				Vector2 p1(tileToScreenCoord(opos));
				Vector2 p2(tileToScreenCoord(p));
				SDL_utils::drawLine(Vector3(p1.x + htw,
							p1.y + htw,
							0.0f),
						Vector3(p2.x + htw,
							p2.y + htw,
							0.0f),
						Color::White, 1.0f);
			}
			opos = p;
		}
	}
}

bool Driver::handleKeyDown(float frameTime, SDLKey key)
{
	return handleKey(frameTime, key, true);
}

bool Driver::handleKeyUp(float frameTime, SDLKey key)
{
	return handleKey(frameTime, key, false);
}

bool Driver::handleMousePress(float frameTime, Uint8 button)
{
	if(button == SDL_BUTTON_LEFT) {
		if(mData.getCurrentTeamID() != mMyTeamID)
			return false;

		if(mGameOver)
			return false;

		auto tgtpos = getMousePosition();
		const MapData* map = mData.getMapData();
		if(tgtpos.x < map->getWidth() &&
				tgtpos.y < map->getHeight()) {
			auto sd = mData.getCurrentSoldier();
			assert(sd);

			auto tgtsoldier = mData.getSoldierAt(tgtpos);
			if(!tgtsoldier) {
				// move
				auto prevpos = sd->position;
				auto l = mAStar.solve(mData.getSoldierPositions(),
						prevpos, tgtpos);
				if(!l.empty()) {
					mPathLine.clear();
					for(auto& p : l) {
						mPathLine.push_back(p);
					}
				}
			} else {
				if(tgtsoldier->teamid != mMyTeamID) {
					// shoot
					shootAt(tgtpos);
				}
			}
		}
	}
	return false;
}

Common::Position Driver::getMousePosition() const
{
	int xp, yp;
	float x, y;
	SDL_GetMouseState(&xp, &yp);

	x = xp / mTileWidth + mCamera.x - (getScreenWidth() / (2.0f * mTileWidth));
	y = 1.0f + yp / mTileWidth + mCamera.y - (getScreenHeight() / (2.0f * mTileWidth));

	return Position(x, y);
}

void Driver::drawGrassTile(unsigned int x, unsigned int y, GrassLevel l)
{
	drawTile(x, y, getTexCoord((unsigned int)l), mGrassTexture);
}

void Driver::drawSoldierTile(unsigned int x, unsigned int y, Common::Direction l, TeamID tid)
{
	drawTile(x, y, getTexCoord((unsigned int)l), mSoldierTextures[tid.id == 1 ? 0 : 1]);
}

void Driver::drawVegetationTile(unsigned int x, unsigned int y, Common::VegetationLevel l)
{
	drawTile(x, y, getTexCoord((unsigned int)l), mVegetationTexture);
}

::Common::Rectangle Driver::getTexCoord(unsigned int i) const
{
	Rectangle tex(0.00f, 0.25f, 0.25f, -0.25f);
	tex.x += (i % 4) * 0.25f;
	tex.y += (i / 4) * 0.25f;
	return tex;
}

::Common::Vector2 Driver::tileToScreenCoord(const Common::Position& p)
{
	return Vector2(mTileWidth * (p.x - mCamera.x) + getScreenWidth() * 0.5f,
			mTileWidth * (mCamera.y - p.y) + getScreenHeight() * 0.5f);
}

void Driver::drawTile(unsigned int x, unsigned int y,
		const ::Common::Rectangle& texcoord,
		const ::Common::Texture* t)
{
	auto s = tileToScreenCoord(Position(x, y));
	SDL_utils::drawSprite(*t, Rectangle(s.x, s.y,
				mTileWidth, mTileWidth), texcoord, 0.0f);
}

bool Driver::handleKey(float frameTime, SDLKey key, bool pressed)
{
	static const float camSpeed = 5.0f;
	static const float camZoomSpeed = 5.0f;
	switch(key) {
		case SDLK_w: mCameraVelocity.y = pressed ? -camSpeed : 0.0f; break;
		case SDLK_s: mCameraVelocity.y = pressed ? camSpeed : 0.0f; break;
		case SDLK_a: mCameraVelocity.x = pressed ? -camSpeed : 0.0f; break;
		case SDLK_d: mCameraVelocity.x = pressed ? camSpeed : 0.0f; break;
		case SDLK_q: mCameraZoomVelocity = pressed ? camZoomSpeed : 0.0f; break;
		case SDLK_e: mCameraZoomVelocity = pressed ? -camZoomSpeed : 0.0f; break;
		case SDLK_KP_ENTER: case SDLK_RETURN: if(pressed) sendEndOfTurn(); break;
		case SDLK_ESCAPE:
			 return true;
		default: break;
	}
	return false;
}

::Common::Color Driver::mapSideColor(bool first, const ::Common::Color& c)
{
	if(c.g >= 128 && c.r == 0 && c.b == 0) {
		Color ret(c);
		if(first) {
			ret.r = c.g;
		} else {
			ret.b = c.g;
		}
		ret.g = 0;
		return ret;
	} else {
		return c;
	}
}

void Driver::updateCurrentSoldier()
{
	Common::QueryResult qr = mWorld.query(Common::CurrentSoldierQuery());
	if(!boost::apply_visitor(mData, qr)) {
		std::cerr << "Current soldier query failed.\n";
		assert(0);
	}
}

void Driver::tryCenterCamera()
{
	const SoldierData* sd = mData.getCurrentSoldier();
	if(sd) {
		const Position& p = sd->position;
		Position tl, br;
		getVisibleMapCoordinates(tl, br);
		const int border = 3;
		if(p.x < tl.x + border || p.x > br.x - border ||
				p.y < tl.y + border || p.y > br.y - border) {
			mCamera.x = p.x;
			mCamera.y = p.y;
		}
	}
}

}

}

