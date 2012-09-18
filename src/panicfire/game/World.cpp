#include <iostream>

#include "panicfire/game/World.h"

namespace PanicFire { 

namespace Game {

using namespace PanicFire::Common;

World::World()
{
	mData = new WorldData(50, 50, 8);
}

World::~World()
{
	delete mData;
}

Common::QueryResult World::query(const Common::Query& q)
{
	return boost::apply_visitor(*this, q);
}

bool World::input(const Common::Input& i)
{
	return false;
}

Common::Event World::pollEvents()
{
	return EmptyEvent();
}

Common::QueryResult World::operator()(const Common::SoldierQuery& q)
{
	SoldierData* sd = mData->getSoldier(q.soldier);
	if(!sd)
		return Common::InvalidQueryResult();

	SoldierQueryResult sqr;
	sqr.soldier = *sd;
	return sqr;
}

Common::QueryResult World::operator()(const Common::MapQuery& q)
{
	MapQueryResult mqr;
	mqr.map = *mData->getMapData();
	return mqr;
}

Common::QueryResult World::operator()(const Common::TeamQuery& q)
{
	TeamData* td = mData->getTeam(q.team);
	if(!td)
		return Common::InvalidQueryResult();

	TeamQueryResult tqr;
	tqr.team = *td;
	return tqr;
}

}

}

