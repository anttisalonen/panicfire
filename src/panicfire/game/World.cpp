#include <iostream>

#include "panicfire/game/World.h"

namespace PanicFire { 

namespace Game {

using namespace PanicFire::Common;

World::World()
{
	mMapData.generate(50, 50);
}

Common::QueryResult World::query(const Common::Query& q)
{
	return PanicFire::Common::DeniedQueryResult();
}

bool World::input(const Common::Input& i)
{
	return false;
}

Common::Event World::pollEvents()
{
	return EmptyEvent();
}

}

}

