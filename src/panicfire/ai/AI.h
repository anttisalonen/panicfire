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

class AI : public boost::static_visitor<> {
	public:
		AI(Common::WorldInterface& w);
		~AI();

		// event handling
		void operator()(const Common::InputEvent& ev);
		void operator()(const Common::SightingEvent& ev);
		void operator()(const Common::EmptyEvent& ev);

		// input event handling
		void operator()(const Common::MovementInput& ev);
		void operator()(const Common::ShotInput& ev);
		void operator()(const Common::FinishTurnInput& ev);

		void act();

	private:
		void sendInput();
		void sendEndOfTurn();
		void handleEvents();
		void updateCurrentSoldier();

		Common::WorldInterface& mWorld;
		Common::WorldData mData;
		UI::AStar mAStar;
		std::list<Common::Position> mPathLine;
		Common::TeamID mMyTeamID;
		bool mMoving;
		Common::Position mMovementPosition;
		Common::SoldierID mCommandedSoldierID;
};

}

}

#endif

