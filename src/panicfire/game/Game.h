#ifndef PANICFIRE_GAME_GAME_H
#define PANICFIRE_GAME_GAME_H

#include "panicfire/common/Structures.h"

namespace PanicFire {

class Game : public Common::GameInterface {

	public:
		Game();
		Common::QueryResult query(const Common::Query& q);
		bool input(const Common::Input& i);
		Common::Event pollEvents();
		void run();

};

}

#endif

