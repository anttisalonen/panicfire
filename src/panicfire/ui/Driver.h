#ifndef PANICFIRE_UI_DRIVER_H
#define PANICFIRE_UI_DRIVER_H

#include <array>

#include "common/Color.h"
#include "common/Vector2.h"
#include "common/Texture.h"
#include "common/DriverFramework.h"

#include "panicfire/common/Structures.h"

#include "panicfire/ai/AI.h"

#include "panicfire/ui/AStar.h"

namespace PanicFire {

namespace UI {

class Drawer {
	public:
		Drawer();
		~Drawer();
		Common::Position getMousePosition() const;
		void drawFrame();
		void drawLine(const Common::Position& p1, const Common::Position& p2);
		void centerCamera();

		void setWorldData(const Common::WorldData* d);
		void setScreenWidth(float w);
		void setScreenHeight(float h);
		void addCameraZoom(float z);
		void moveCamera(const ::Common::Vector2& v);

	private:
		void drawGrassTile(unsigned int x, unsigned int y, Common::GrassLevel l);
		void drawSpot(unsigned int x, unsigned int y);
		void drawVegetationTile(unsigned int x, unsigned int y, Common::VegetationLevel l);
		void drawSoldierTile(unsigned int x, unsigned int y, Common::Direction l,
				Common::TeamID tid);
		static ::Common::Rectangle getTexCoord(unsigned int i);
		void drawTile(unsigned int x, unsigned int y,
				const ::Common::Rectangle& texcoord,
				const ::Common::Texture* t);
		::Common::Vector2 tileToScreenCoord(const Common::Position& p);
		void getVisibleMapCoordinates(Common::Position& tl, Common::Position& br) const;
		::Common::Color mapSideColor(bool first, const ::Common::Color& c) const;

		float mCameraZoom;
		::Common::Vector2 mCamera;
		::Common::Texture* mGrassTexture;
		::Common::Texture* mVegetationTexture;
		::Common::Texture* mSpotTexture;
		std::array< ::Common::Texture*, 2> mSoldierTextures;
		float mTileWidth;
		const Common::WorldData* mWorldData;
		float mScreenWidth;
		float mScreenHeight;
};

class Driver : public ::Common::Driver, public boost::static_visitor<> {
	public:
		Driver(Common::WorldInterface& w);
		~Driver();

		// event handling
		void operator()(const Common::InputEvent& ev);
		void operator()(const Common::SightingEvent& ev);
		void operator()(const Common::SoldierWoundedEvent& ev);
		void operator()(const Common::GameWonEvent& ev);
		void operator()(const Common::EmptyEvent& ev);

		// input event handling
		void operator()(const Common::MovementInput& ev);
		void operator()(const Common::ShotInput& ev);
		void operator()(const Common::FinishTurnInput& ev);

	protected:
		bool init() override;
		bool prerenderUpdate(float frameTime) override;
		void drawFrame() override;
		bool handleKeyDown(float frameTime, SDLKey key) override;
		bool handleKeyUp(float frameTime, SDLKey key) override;
		bool handleMousePress(float frameTime, Uint8 button) override;

	private:
		bool handleKey(float frameTime, SDLKey key, bool pressed);
		void sendInput();
		void sendEndOfTurn();
		void handleEvents();
		void updateCurrentSoldier();
		void shootAt(const Common::Position& tgtpos);

		Common::WorldInterface& mWorld;
		Common::WorldData mData;
		Drawer mDrawer;
		::Common::Vector2 mCameraVelocity;
		float mCameraZoomVelocity;
		AStar mAStar;
		std::list<Common::Position> mPathLine;
		Common::TeamID mMyTeamID;
		bool mMoving;
		Common::Position mMovementPosition;
		Common::SoldierID mCommandedSoldierID;
		AI::AI mAI;
		bool mGameOver;
};

}

}

#endif

