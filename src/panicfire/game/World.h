#ifndef PANICFIRE_GAME_GAME_H
#define PANICFIRE_GAME_GAME_H

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

	private:
		Common::WorldData *mData;
};

}

}

#endif

