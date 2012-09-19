#ifndef PANICFIRE_UI_DRIVER_H
#define PANICFIRE_UI_DRIVER_H

#include <array>

#include "common/Color.h"
#include "common/Vector2.h"
#include "common/Texture.h"
#include "common/DriverFramework.h"

#include "panicfire/common/Structures.h"

namespace PanicFire {

namespace UI {

class Driver;

class Driver : public ::Common::Driver {
	public:
		Driver(Common::WorldInterface& w);
		~Driver();

	protected:
		bool init() override;
		bool prerenderUpdate(float frameTime) override;
		void drawFrame() override;
		bool handleKeyDown(float frameTime, SDLKey key) override;
		bool handleKeyUp(float frameTime, SDLKey key) override;

	private:
		void drawGrassTile(unsigned int x, unsigned int y, Common::GrassLevel l);
		void drawVegetationTile(unsigned int x, unsigned int y, Common::VegetationLevel l);
		void drawSoldierTile(unsigned int x, unsigned int y, Common::Direction l,
				Common::TeamID tid);
		::Common::Rectangle getTexCoord(unsigned int i) const;
		void drawTile(unsigned int x, unsigned int y,
				const ::Common::Rectangle& texcoord,
				const ::Common::Texture* t);
		bool handleKey(float frameTime, SDLKey key, bool pressed);
		::Common::Color mapSideColor(bool first, const ::Common::Color& c);

		Common::WorldInterface& mWorld;
		Common::WorldData mData;
		::Common::Vector2 mCamera;
		::Common::Vector2 mCameraVelocity;
		float mCameraZoom;
		float mCameraZoomVelocity;
		float mTileWidth;
		::Common::Texture* mGrassTexture;
		::Common::Texture* mVegetationTexture;
		std::array< ::Common::Texture*, 2> mSoldierTextures;
};

}

}

#endif

