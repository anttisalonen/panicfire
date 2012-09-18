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
}

Driver::~Driver()
{
	delete mGrassTexture;
	delete mVegetationTexture;
}

bool Driver::init()
{
	SDL_utils::setupOrthoScreen(getScreenWidth(), getScreenHeight());
	Common::QueryResult qr = mWorld.query(Common::MapQuery());
	boost::apply_visitor(*this, qr);
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
	unsigned int maxy = std::min(mMap.getHeight(), (unsigned int)(mCamera.y + mCameraZoom + 1));
	unsigned int maxx = std::min(mMap.getWidth(),  (unsigned int)(mCamera.x + mCameraZoom + 1));
	for(unsigned int j = miny; j < maxy; j++) {
		for(unsigned int i = minx; i < maxx; i++) {
			auto fr = mMap.getPoint(i, j);
			drawGrassTile(i, j, fr.grasslevel);
		}
	}

	for(unsigned int j = miny; j < maxy; j++) {
		for(unsigned int i = minx; i < maxx; i++) {
			auto fr = mMap.getPoint(i, j);
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

void Driver::operator()(const Common::SoldierQueryResult& q)
{
	std::cout << "Soldier query successful.\n";
}

void Driver::operator()(const Common::TeamQueryResult& q)
{
	std::cout << "Team query successful.\n";
}

void Driver::operator()(const Common::MapQueryResult& q)
{
	std::cout << "Map query successful.\n";
	mMap = q.map;
}

void Driver::operator()(const Common::DeniedQueryResult& q)
{
	std::cerr << "Driver error: denied query.\n";
}

void Driver::operator()(const Common::InvalidQueryResult& q)
{
	std::cerr << "Driver error: invalid query.\n";
}

void Driver::drawGrassTile(unsigned int x, unsigned int y, GrassLevel l)
{
	drawTile(x, y, getTexCoord((unsigned int)l), mGrassTexture);
}

void Driver::drawVegetationTile(unsigned int x, unsigned int y, Common::VegetationLevel l)
{
	drawTile(x, y, getTexCoord((unsigned int)l), mVegetationTexture);
}

::Common::Rectangle Driver::getTexCoord(unsigned int i) const
{
	Rectangle tex(0.25f, 0.25f, -0.25f, -0.25f);
	tex.x += (i % 4) * 0.25f;
	tex.y += (i / 4) * 0.25f;
	return tex;
}

void Driver::drawTile(unsigned int x, unsigned int y,
		const ::Common::Rectangle& texcoord,
		const ::Common::Texture* t)
{
	SDL_utils::drawSprite(*t, Rectangle(mTileWidth * (x - mCamera.x) + getScreenWidth() * 0.5f,
				mTileWidth * (y - mCamera.y) + getScreenHeight() * 0.5f,
				mTileWidth, mTileWidth), texcoord, 0.0f);
}

bool Driver::handleKey(float frameTime, SDLKey key, bool pressed)
{
	static const float camSpeed = 5.0f;
	static const float camZoomSpeed = 5.0f;
	switch(key) {
		case SDLK_w: mCameraVelocity.y = pressed ? camSpeed : 0.0f; break;
		case SDLK_s: mCameraVelocity.y = pressed ? -camSpeed : 0.0f; break;
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


}

}

