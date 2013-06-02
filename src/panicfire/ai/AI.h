#ifndef PANICFIRE_AI_AI_H
#define PANICFIRE_AI_AI_H

#include <array>

#include "common/Color.h"
#include "common/Vector2.h"
#include "common/Texture.h"
#include "common/DriverFramework.h"

#include "panicfire/common/Structures.h"

#include "panicfire/ui/AStar.h"

namespace PanicFire {

namespace AI {

struct AIData;

class TeamPlan {
	public:
		TeamPlan();
		void setAIData(AIData* data);
		void positionVisited(const Common::Position& p);
		Common::Position getNextVisitPosition() const;

	private:
		AIData* mAIData;
		mutable std::set<Common::Position> mVisitPositions;
};

class SoldierPlan : public boost::static_visitor<> {
	public:
		SoldierPlan(AIData& d, Common::SoldierID i);

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

		void act();

	private:
		void syncSoldierData();
		void handleEvents();
		void setupPath();
		void sendInput();
		void checkShotChance();

		AIData& mAIData;
		Common::SoldierID mID;
		bool mSentInput;
		Common::Position mTargetPosition;
		Common::Position mShootPosition;
		bool mShooting;
		std::list<Common::Position> mPath;
};

struct AIData {
	AIData(Common::WorldInterface& w);
	void updateCurrentSoldier();

	Common::WorldInterface& mWorld;
	Common::WorldData mData;
	std::map<Common::SoldierID, SoldierPlan> mSoldierPlan;
	UI::AStar mAStar;
	Common::TeamID mMyTeamID;
	bool mGameOver;
	TeamPlan mTeamPlan;
	bool mMyTurn;
};

class AI {
	public:
		AI(Common::WorldInterface& w);
		~AI();

		void act();

	private:
		void sendInput();
		void sendEndOfTurn();

		AIData mAIData;
		std::list<Common::Position> mPathLine;
		Common::Position mMovementPosition;
		Common::SoldierID mCommandedSoldierID;

};

}

}

#endif

