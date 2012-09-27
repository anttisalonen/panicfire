#include <iostream>

#include "common/Math.h"
#include "common/Rectangle.h"
#include "common/SDL_utils.h"

#include "panicfire/ui/Driver.h"

using namespace Common;
using namespace PanicFire;
using namespace PanicFire::Common;

namespace PanicFire {

namespace UI {

Animation::Animation(const Common::Position& start, float s)
	: speed(s),
	pos(0.0f)
{
	addPosition(start);
}

bool Animation::finished() const
{
	return path.size() < 2;
}

::Common::Vector2 Animation::getPosition() const
{
	if(path.empty())
		return Vector2(0, 0);

	if(path.size() == 1) {
		auto& p = *path.begin();
		return Vector2(p.x, p.y);
	}

	auto& p1 = *path.begin();
	auto& p2 = *(++path.begin());
	Vector2 vp1(p1.x, p1.y);
	Vector2 vp2(p2.x, p2.y);
	Vector2 off = (vp2 - vp1) * pos;
	Vector2 r = vp1 + off;
	return r;
}

void Animation::addPosition(const Common::Position& p)
{
	path.push_back(p);
}

void Animation::advance(float p)
{
	pos += p * speed;
	while(pos > 1.0f && !path.empty()) {
		path.pop_front();
		pos -= 1.0f;
	}
}

SoldierAnimation::SoldierAnimation(SoldierID i, const Common::Position& start, float s)
	: Animation(start, s),
	id(i)
{
}

BulletAnimation::BulletAnimation(const Common::Position& f, const Common::Position& t, float s)
	: Animation(f, s),
	target(t)
{
	addPosition(t);
}

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

void Drawer::advanceAnimation(float ft)
{
	for(std::map<SoldierID, SoldierAnimation>::iterator ait = mSoldierAnimation.begin();
			ait != mSoldierAnimation.end(); ) {
		ait->second.advance(ft);
		if(ait->second.finished()) {
			ait = mSoldierAnimation.erase(ait);
			if(mSoldierAnimation.empty() && mBulletAnimation.empty()) {
				centerCamera();
			}
		} else {
			++ait;
		}
	}

	for(std::list<BulletAnimation>::iterator ait = mBulletAnimation.begin();
			ait != mBulletAnimation.end(); ) {
		ait->advance(ft);
		if(ait->finished()) {
			ait = mBulletAnimation.erase(ait);
		} else {
			++ait;
		}
	}
}

void Drawer::addSoldierAnimation(SoldierID i, const Common::Position& from,
		const Common::Position& to)
{
	auto ait = mSoldierAnimation.find(i);
	if(ait == mSoldierAnimation.end()) {
		SoldierAnimation t(i, from, 2.0f);
		t.addPosition(to);
		mSoldierAnimation.insert({i, t});
	} else {
		ait->second.addPosition(to);
	}
}

void Drawer::addBulletAnimation(const Common::Position& from, const Common::Position& to)
{
	mBulletAnimation.push_back(BulletAnimation(from, to, 4.0f));
}

bool Drawer::isAnimationRunning() const
{
	return !mSoldierAnimation.empty() || !mBulletAnimation.empty();
}

void Drawer::drawSoldierAnimation(const SoldierAnimation& a)
{
	Vector2 pos = a.getPosition();
	const SoldierData* sd = mWorldData->getSoldier(a.id);
	assert(sd);
	drawSoldierTile(pos, sd->direction, sd->teamid);
}

::Common::Rectangle Drawer::getTexCoord(unsigned int i)
{
	Rectangle tex(0.00f, 0.25f, 0.25f, -0.25f);
	tex.x += (i % 4) * 0.25f;
	tex.y += (i / 4) * 0.25f;
	return tex;
}

::Common::Vector2 Drawer::tileToScreenCoord(const ::Common::Vector2& p)
{
	return Vector2(mTileWidth * (p.x - mCamera.x) + mScreenWidth * 0.5f,
			mTileWidth * (mCamera.y - p.y) + mScreenHeight * 0.5f);
}

::Common::Vector2 Drawer::tileToScreenCoord(const Common::Position& p)
{
	return tileToScreenCoord(Vector2(p.x, p.y));
}

void Drawer::drawGrassTile(unsigned int x, unsigned int y, GrassLevel l)
{
	drawTile(Position(x, y), getTexCoord((unsigned int)l), mGrassTexture);
}

void Drawer::drawSpot(unsigned int x, unsigned int y)
{
	drawTile(Position(x, y), Rectangle(0, 0, 1, 1), mSpotTexture);
}

void Drawer::drawSoldierTile(const ::Common::Vector2& p, Common::Direction l,
		Common::TeamID tid)
{
	drawTile(p, getTexCoord((unsigned int)l), mSoldierTextures[tid.id == 1 ? 0 : 1]);
}

void Drawer::drawSoldierTile(const Position& p, Common::Direction l, TeamID tid)
{
	drawTile(p, getTexCoord((unsigned int)l), mSoldierTextures[tid.id == 1 ? 0 : 1]);
}

void Drawer::drawVegetationTile(unsigned int x, unsigned int y, Common::VegetationLevel l)
{
	drawTile(Position(x, y), getTexCoord((unsigned int)l), mVegetationTexture);
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

void Drawer::drawBullet(const ::Common::Vector2& p)
{
	Vector2 b(p.x + 0.5f, p.y - 0.5f);
	auto s = tileToScreenCoord(b);
	SDL_utils::drawPoint(Vector3(s.x, s.y, 0.0), mTileWidth * 0.1f, Color::White);
}

void Drawer::drawTile(const ::Common::Vector2& p,
		const ::Common::Rectangle& texcoord,
		const ::Common::Texture* t)
{
	auto s = tileToScreenCoord(p);
	SDL_utils::drawSprite(*t, Rectangle(s.x, s.y,
				mTileWidth, mTileWidth), texcoord, 0.0f);
}

void Drawer::drawTile(const Common::Position& p,
		const ::Common::Rectangle& texcoord,
		const ::Common::Texture* t)
{
	return drawTile(Vector2(p.x, p.y), texcoord, t);
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
						bool alive = sd->health.value > 0;
						bool bulletapproaching = false;
						for(auto b : mBulletAnimation) {
							if(b.target == sd->position) {
								bulletapproaching = true;
								break;
							}
						}

						if((alive || bulletapproaching) &&
								p.x >= minx && p.x < maxx &&
								p.y >= miny && p.y < maxy) {
							auto ait = mSoldierAnimation.find(sid);
							if(ait == mSoldierAnimation.end()) {
								if(mWorldData->getCurrentSoldierID() == sid) {
									drawSpot(p.x, p.y);
								}
								drawSoldierTile(p, sd->direction, td->id);
							} else {
								drawSoldierAnimation(ait->second);
							}
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

	for(auto& b : mBulletAnimation) {
		drawBullet(b.getPosition());
	}
}

SoldierID Drawer::getAnimatedSoldier() const
{
	if(mSoldierAnimation.empty())
		return SoldierID(0);
	else
		return mSoldierAnimation.begin()->second.id;
}

void Drawer::centerCamera()
{
	SoldierID sid;
	sid = getAnimatedSoldier();
	if(sid.id == 0)
		sid = mWorldData->getCurrentSoldierID();

	const SoldierData* sd = mWorldData->getSoldier(sid);
	if(!sd)
		return;

	Vector2 p;
	auto ait = mSoldierAnimation.find(sd->id);
	if(ait == mSoldierAnimation.end())
		p = Vector2(sd->position.x, sd->position.y);
	else
		p = ait->second.getPosition();

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
	handleEvents();
	sendInput();
	handleEvents(); // handle response to input

	mDrawer.moveCamera(mCameraVelocity * frameTime);
	mDrawer.addCameraZoom(mCameraZoomVelocity * frameTime);
	mDrawer.advanceAnimation(frameTime);

	if(mDrawer.isAnimationRunning()) {
		mDrawer.centerCamera();
	}

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

	if(mData.getCurrentTeamID() != mMyTeamID)
		return;

	{
		auto sid = mDrawer.getAnimatedSoldier();
		if(sid.id != 0 && mData.teamIDFromSoldierID(sid) != mMyTeamID)
			return;
	}

	if(!mPathLine.empty()) {
		auto sd = mData.getCurrentSoldier();
		for(auto pit = mPathLine.begin(); pit != mPathLine.end(); ) {
			if(sd.position == *pit) {
				pit = mPathLine.erase(pit);
			} else {
				MovementInput i(mData.getCurrentSoldierID(), sd.position, *pit);
				if(mData.movementAllowed(i)) {
					bool succ = mWorld.input(i);
					assert(succ);
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

	if(mDrawer.isAnimationRunning())
		return;

	bool succ = mWorld.input(FinishTurnInput());
	assert(succ);
	mPathLine.clear();
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
	mDrawer.addSoldierAnimation(ev.mover, ev.from, ev.to);
}

void Driver::operator()(const Common::ShotInput& ev)
{
	auto sd = mData.getSoldier(ev.shooter);
	if(sd) {
		auto from = sd->position;
		mDrawer.addBulletAnimation(from, ev.target);
	}
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

