#ifndef PANICFIRE_UI_DRIVER_H
#define PANICFIRE_UI_DRIVER_H

#include "common/DriverFramework.h"
#include "common/Texture.h"
#include "common/Vector2.h"

#include "panicfire/common/Structures.h"

namespace PanicFire {

namespace UI {

class Driver;

class Driver : public ::Common::Driver, public boost::static_visitor<> {
	public:
		Driver(Common::WorldInterface& w);
		~Driver();
		void operator()(const Common::SoldierQueryResult& q);
		void operator()(const Common::TeamQueryResult& q);
		void operator()(const Common::MapQueryResult& q);
		void operator()(const Common::DeniedQueryResult& q);
		void operator()(const Common::InvalidQueryResult& q);

	protected:
		bool init() override;
		bool prerenderUpdate(float frameTime) override;
		void drawFrame() override;
		bool handleKeyDown(float frameTime, SDLKey key) override;
		bool handleKeyUp(float frameTime, SDLKey key) override;

	private:
		void drawGrassTile(unsigned int x, unsigned int y, Common::GrassLevel l);
		void drawVegetationTile(unsigned int x, unsigned int y, Common::VegetationLevel l);
		::Common::Rectangle getTexCoord(unsigned int i) const;
		void drawTile(unsigned int x, unsigned int y,
				const ::Common::Rectangle& texcoord,
				const ::Common::Texture* t);
		bool handleKey(float frameTime, SDLKey key, bool pressed);

		Common::WorldInterface& mWorld;
		Common::MapData mMap;
		::Common::Vector2 mCamera;
		::Common::Vector2 mCameraVelocity;
		float mCameraZoom;
		float mCameraZoomVelocity;
		float mTileWidth;
		::Common::Texture* mGrassTexture;
		::Common::Texture* mVegetationTexture;
};

}

}

#endif

