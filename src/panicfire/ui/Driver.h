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

class Animation {
	public:
		Animation(const Common::Position& start, float s);
		virtual ~Animation() { }
		::Common::Vector2 getPosition() const;
		void addPosition(const Common::Position& p);
		void advance(float p);
		bool finished() const;
	protected:
		std::list<Common::Position> path;
		float speed;
		float pos;
};

struct SoldierAnimation : public Animation {
	SoldierAnimation(Common::SoldierID i, const Common::Position& start, float s);

	Common::SoldierID id;
};

struct BulletAnimation : public Animation {
	BulletAnimation(const Common::Position& f, const Common::Position& t, float s);
	Common::Position target;
};

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
		void advanceAnimation(float ft);
		void addSoldierAnimation(Common::SoldierID i, const Common::Position& from,
				const Common::Position& to);
		bool isAnimationRunning() const;
		Common::SoldierID getAnimatedSoldier() const;
		void addBulletAnimation(const Common::Position& from, const Common::Position& to);

	private:
		void drawGrassTile(unsigned int x, unsigned int y, Common::GrassLevel l);
		void drawSpot(unsigned int x, unsigned int y);
		void drawVegetationTile(unsigned int x, unsigned int y, Common::VegetationLevel l);
		void drawSoldierTile(const ::Common::Vector2& p, Common::Direction l,
				Common::TeamID tid);
		void drawSoldierTile(const Common::Position& p, Common::Direction l,
				Common::TeamID tid);
		void drawBullet(const ::Common::Vector2& p);
		static ::Common::Rectangle getTexCoord(unsigned int i);
		void drawTile(const ::Common::Vector2& p,
				const ::Common::Rectangle& texcoord,
				const ::Common::Texture* t);
		void drawTile(const Common::Position& p,
				const ::Common::Rectangle& texcoord,
				const ::Common::Texture* t);
		::Common::Vector2 tileToScreenCoord(const Common::Position& p);
		::Common::Vector2 tileToScreenCoord(const ::Common::Vector2& p);
		void getVisibleMapCoordinates(Common::Position& tl, Common::Position& br) const;
		::Common::Color mapSideColor(bool first, const ::Common::Color& c) const;
		void drawSoldierAnimation(const SoldierAnimation& a);

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

		std::map<Common::SoldierID, SoldierAnimation> mSoldierAnimation;
		std::list<BulletAnimation> mBulletAnimation;
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
		Common::Position mMovementPosition;
		Common::SoldierID mCommandedSoldierID;
		AI::AI mAI;
		bool mGameOver;
};

}

}

#endif

