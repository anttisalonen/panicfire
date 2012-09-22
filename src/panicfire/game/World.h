#ifndef PANICFIRE_GAME_GAME_H
#define PANICFIRE_GAME_GAME_H

#include <array>
#include <queue>

#include "panicfire/common/Structures.h"

namespace PanicFire {

namespace Game {

class World : public Common::WorldInterface,
	public boost::static_visitor<Common::QueryResult> {

	public:
		World();
		~World();

		Common::QueryResult query(const Common::Query& q);
		bool input(const Common::Input& i);
		Common::Event pollEvents();

		Common::QueryResult operator()(const Common::SoldierQuery& q);
		Common::QueryResult operator()(const Common::MapQuery& q);
		Common::QueryResult operator()(const Common::TeamQuery& q);
		Common::QueryResult operator()(const Common::CurrentSoldierQuery& q);

		Common::QueryResult operator()(const Common::MovementInput& i);
		Common::QueryResult operator()(const Common::ShotInput& i);
		Common::QueryResult operator()(const Common::FinishTurnInput& i);

	private:
		Common::WorldData *mData;
		std::array<std::queue<Common::Event>, MAX_NUM_TEAMS> mEventQueue;
};

}

}

#endif

