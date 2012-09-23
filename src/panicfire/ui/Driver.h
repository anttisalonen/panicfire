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
		::Common::Vector2 tileToScreenCoord(const Common::Position& p);
		Common::Position getMousePosition() const;
		void sendInput();
		void sendEndOfTurn();
		void handleEvents();
		void updateCurrentSoldier();
		void tryCenterCamera();
		void getVisibleMapCoordinates(Common::Position& tl, Common::Position& br) const;
		void shootAt(const Common::Position& tgtpos);

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

