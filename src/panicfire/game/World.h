#ifndef PANICFIRE_GAME_GAME_H
#define PANICFIRE_GAME_GAME_H

#include "panicfire/common/Structures.h"

namespace PanicFire {

namespace Game {

class World : public Common::WorldInterface {

	public:
		World();

		Common::QueryResult query(const Common::Query& q);
		bool input(const Common::Input& i);
		Common::Event pollEvents();

	private:
		Common::MapData mMapData;
};

}

}

#endif

