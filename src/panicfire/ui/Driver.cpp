#include <iostream>

#include "common/Rectangle.h"
#include "common/SDL_utils.h"

#include "panicfire/ui/Driver.h"

using namespace Common;
using namespace PanicFire;
using namespace PanicFire::Common;

namespace PanicFire {

namespace UI {

Drawer::Drawer()
	: mCameraZoom(10.0f),
	mTileWidth(10.0f),
	mWorldData(nullptr),
	mScreenWidth(10.0f),
	mScreenHeight(10.0f)
{
	mCamera.x = mCameraZoom;
	mCamera.y = mCameraZoom;
	mGrassTexture = new Texture("share/grass.png", 0, 0);
	mVegetationTexture = new Texture("share/vegetation.png", 0, 0);
	mSpotTexture = new Texture("share/spot.png", 0, 0);

	SDLSurface soldsurf("share/soldier.png");
	for(int i = 0; i < 2; i++) {
		SDLSurface surf(soldsurf);
		surf.mapPixelColor( [&] (const Color& c) { return mapSideColor(i == 0, c); } );
		mSoldierTextures[i] = new Texture(surf, 0, 0);
	}
}

Drawer::~Drawer()
{
	delete mSpotTexture;
	delete mVegetationTexture;
	delete mGrassTexture;
	for(auto t : mSoldierTextures)
		delete t;
}

void Drawer::setWorldData(const Common::WorldData* d)
{
	mWorldData = d;
}

void Drawer::setScreenWidth(float w)
{
	mScreenWidth = w;
}

void Drawer::setScreenHeight(float h)
{
	mScreenHeight = h;
}

void Drawer::addCameraZoom(float z)
{
	mCameraZoom += z;
	mTileWidth = std::max(mScreenWidth, mScreenHeight) / (mCameraZoom * 2.0f);
}

void Drawer::moveCamera(const Vector2& v)
{
	mCamera += v;
}

::Common::Rectangle Drawer::getTexCoord(unsigned int i)
{
	Rectangle tex(0.00f, 0.25f, 0.25f, -0.25f);
	tex.x += (i % 4) * 0.25f;
	tex.y += (i / 4) * 0.25f;
	return tex;
}

::Common::Vector2 Drawer::tileToScreenCoord(const Common::Position& p)
{
	return Vector2(mTileWidth * (p.x - mCamera.x) + mScreenWidth * 0.5f,
			mTileWidth * (mCamera.y - p.y) + mScreenHeight * 0.5f);
}

void Drawer::drawGrassTile(unsigned int x, unsigned int y, GrassLevel l)
{
	drawTile(x, y, getTexCoord((unsigned int)l), mGrassTexture);
}

void Drawer::drawSpot(unsigned int x, unsigned int y)
{
	drawTile(x, y, Rectangle(0, 0, 1, 1), mSpotTexture);
}

void Drawer::drawSoldierTile(unsigned int x, unsigned int y, Common::Direction l, TeamID tid)
{
	drawTile(x, y, getTexCoord((unsigned int)l), mSoldierTextures[tid.id == 1 ? 0 : 1]);
}

void Drawer::drawVegetationTile(unsigned int x, unsigned int y, Common::VegetationLevel l)
{
	drawTile(x, y, getTexCoord((unsigned int)l), mVegetationTexture);
}

void Drawer::drawLine(const Common::Position& p1, const Common::Position& p2)
{
	float htw = mTileWidth * 0.5f;
	Vector2 vp1(tileToScreenCoord(p1));
	Vector2 vp2(tileToScreenCoord(p2));
	SDL_utils::drawLine(Vector3(vp1.x + htw,
				vp1.y + htw,
				0.0f),
			Vector3(vp2.x + htw,
				vp2.y + htw,
				0.0f),
			Color::White, 1.0f);
}

void Drawer::drawTile(unsigned int x, unsigned int y,
		const ::Common::Rectangle& texcoord,
		const ::Common::Texture* t)
{
	auto s = tileToScreenCoord(Position(x, y));
	SDL_utils::drawSprite(*t, Rectangle(s.x, s.y,
				mTileWidth, mTileWidth), texcoord, 0.0f);
}

Common::Position Drawer::getMousePosition() const
{
	int xp, yp;
	float x, y;
	SDL_GetMouseState(&xp, &yp);

	x = xp / mTileWidth + mCamera.x - (mScreenWidth / (2.0f * mTileWidth));
	y = 1.0f + yp / mTileWidth + mCamera.y - (mScreenHeight / (2.0f * mTileWidth));

	return Position(x, y);
}

void Drawer::drawFrame()
{
	Position tl, br;
	getVisibleMapCoordinates(tl, br);
	unsigned int minx, miny, maxx, maxy;
	minx = tl.x; miny = tl.y; maxx = br.x; maxy = br.y;

	const MapData* map = mWorldData->getMapData();

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
		const TeamData* td = mWorldData->getTeam(TeamID(t));
		assert(td);
		if(td) {
			for(unsigned int s = 0; s < MAX_TEAM_SOLDIERS; s++) {
				SoldierID sid = td->soldiers[s];
				if(sid.id) {
					const SoldierData* sd = mWorldData->getSoldier(sid);
					assert(sd);
					if(sd) {
						const Position& p = sd->position;
						if(sd->health.value > 0 &&
								p.x >= minx && p.x < maxx &&
								p.y >= miny && p.y < maxy) {
							if(mWorldData->getCurrentSoldierID() == sid) {
								drawSpot(p.x, p.y);
							}
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
}

void Drawer::centerCamera()
{
	const SoldierData& sd = mWorldData->getCurrentSoldier();
	const Position& p = sd.position;
	Position tl, br;
	getVisibleMapCoordinates(tl, br);
	const int border = 3;
	if(p.x < tl.x + border || p.x > br.x - border ||
			p.y < tl.y + border || p.y > br.y - border) {
		mCamera.x = p.x;
		mCamera.y = p.y;
	}
}

void Drawer::getVisibleMapCoordinates(Position& tl, Position& br) const
{
	unsigned int miny = std::max(0.0f, mCamera.y - mCameraZoom);
	unsigned int minx = std::max(0.0f, mCamera.x - mCameraZoom);
	const MapData* map = mWorldData->getMapData();
	unsigned int maxy = std::min(map->getHeight(), (unsigned int)(mCamera.y + mCameraZoom + 1));
	unsigned int maxx = std::min(map->getWidth(),  (unsigned int)(mCamera.x + mCameraZoom + 1));
	tl.x = minx;
	tl.y = miny;
	br.x = maxx;
	br.y = maxy;
}

::Common::Color Drawer::mapSideColor(bool first, const ::Common::Color& c) const
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

// driver
Driver::Driver(Common::WorldInterface& w)
	: ::Common::Driver(800, 600, "Panic Fire"),
	mWorld(w),
	mCameraZoomVelocity(0.0f),
	mMyTeamID(TeamID(1)),
	mMoving(false),
	mAI(w),
	mGameOver(false)
{
}

Driver::~Driver()
{
}

bool Driver::init()
{
	SDL_utils::setupOrthoScreen(getScreenWidth(), getScreenHeight());
	if(!mData.sync(mWorld))
		return false;

	mAStar.setMapData(mData.getMapData());
	mDrawer.setWorldData(&mData);

	mDrawer.centerCamera();
	mDrawer.setScreenWidth(getScreenWidth());
	mDrawer.setScreenHeight(getScreenHeight());

	return true;
}

bool Driver::prerenderUpdate(float frameTime)
{
	mDrawer.moveCamera(mCameraVelocity * frameTime);
	mDrawer.addCameraZoom(mCameraZoomVelocity * frameTime);

	handleEvents();
	sendInput();

	return false;
}

void Driver::drawFrame()
{
	mDrawer.drawFrame();

	Position opos(UINT_MAX, 0);
	for(auto& p : mPathLine) {
		if(opos.x != UINT_MAX) {
			mDrawer.drawLine(opos, p);
		}
		opos = p;
	}
}

void Driver::sendInput()
{
	if(mGameOver)
		return;

	if(!mPathLine.empty() && !mMoving) {
		auto sd = mData.getCurrentSoldier();
		for(auto pit = mPathLine.begin(); pit != mPathLine.end(); ) {
			if(sd.position == *pit) {
				pit = mPathLine.erase(pit);
			} else {
				MovementInput i(mData.getCurrentSoldierID(), *pit);
				if(mData.movementAllowed(i)) {
					bool succ = mWorld.input(i);
					assert(succ);
					mMoving = true;
					mMovementPosition = *pit;
					mCommandedSoldierID = sd.id;
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
	mDrawer.centerCamera();
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
	mDrawer.centerCamera();
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

		auto tgtpos = mDrawer.getMousePosition();
		const MapData* map = mData.getMapData();
		if(tgtpos.x < map->getWidth() &&
				tgtpos.y < map->getHeight()) {
			auto sd = mData.getCurrentSoldier();

			auto tgtsoldier = mData.getSoldierAt(tgtpos);
			if(!tgtsoldier) {
				// move
				auto prevpos = sd.position;
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

void Driver::updateCurrentSoldier()
{
	mData.syncCurrentSoldier(mWorld);
}

}

}

