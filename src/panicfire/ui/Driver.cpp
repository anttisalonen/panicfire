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
	mCameraZoomVelocity(0.0f)
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
	bool cameraInit = false;

	SDL_utils::setupOrthoScreen(getScreenWidth(), getScreenHeight());
	{
		Common::QueryResult qr = mWorld.query(Common::MapQuery());
		if(!boost::apply_visitor(mData, qr)) {
			std::cerr << "Map query failed.\n";
			return false;
		}
	}

	for(unsigned int i = 1; i <= MAX_NUM_TEAMS; i++) {
		TeamID tid = TeamID(i);
		Common::QueryResult qr = mWorld.query(Common::TeamQuery(tid));
		if(!boost::apply_visitor(mData, qr)) {
			std::cerr << "Team query failed for team " << tid.id << ".\n";
			return false;
		}
		TeamData* td = mData.getTeam(tid);
		assert(td);
		if(td) {
			for(unsigned int j = 0; j < MAX_TEAM_SOLDIERS; j++) {
				if(!td->soldiers[j].id)
					continue;
				SoldierID sid = td->soldiers[j];
				Common::QueryResult qr2 = mWorld.query(Common::SoldierQuery(sid));
				if(!boost::apply_visitor(mData, qr2)) {
					std::cerr << "Soldier query failed for soldier " << sid.id << ".\n";
					return false;
				} else if(!cameraInit) {
					if(td->id.id == 1) {
						const SoldierData* sd = mData.getSoldier(sid);
						assert(sd);
						const Position& p = sd->position;
						mCamera.x = p.x;
						mCamera.y = p.y;
					}
				}
			}
		}
	}
	return true;
}

bool Driver::prerenderUpdate(float frameTime)
{
	mCamera += mCameraVelocity * frameTime;
	mCameraZoom += mCameraZoomVelocity * frameTime;
	mTileWidth = std::max(getScreenWidth(), getScreenHeight()) / (mCameraZoom * 2.0f);
	return false;
}

void Driver::drawFrame()
{
	unsigned int miny = std::max(0.0f, mCamera.y - mCameraZoom);
	unsigned int minx = std::max(0.0f, mCamera.x - mCameraZoom);
	const MapData* map = mData.getMapData();
	unsigned int maxy = std::min(map->getHeight(), (unsigned int)(mCamera.y + mCameraZoom + 1));
	unsigned int maxx = std::min(map->getWidth(),  (unsigned int)(mCamera.x + mCameraZoom + 1));
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
						if(p.x >= minx && p.x < maxx &&
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
}

bool Driver::handleKeyDown(float frameTime, SDLKey key)
{
	return handleKey(frameTime, key, true);
}

bool Driver::handleKeyUp(float frameTime, SDLKey key)
{
	return handleKey(frameTime, key, false);
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

void Driver::drawTile(unsigned int x, unsigned int y,
		const ::Common::Rectangle& texcoord,
		const ::Common::Texture* t)
{
	SDL_utils::drawSprite(*t, Rectangle(mTileWidth * (x - mCamera.x) + getScreenWidth() * 0.5f,
				mTileWidth * (mCamera.y - y) + getScreenHeight() * 0.5f,
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


}

}

