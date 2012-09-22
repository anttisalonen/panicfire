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
	// hack: we're boost::static_visitor<Common::QueryResult>,
	// so convert query result to bool.
	// invalid => true, denied => false.
	auto qr = boost::apply_visitor(*this, i);
	return boost::get<DeniedQueryResult>(&qr) == 0;
}

Common::Event World::pollEvents(TeamID tid)
{
	/* TODO: check client */
	auto tindex = mData->teamIndexFromTeamID(tid);
	auto& q = mEventQueue[tindex];
	if(q.empty()) {
		return EmptyEvent();
	} else {
		auto ev = q.front();
		q.pop();
		return ev;
	}
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

Common::QueryResult World::operator()(const Common::CurrentSoldierQuery& q)
{
	CurrentSoldierQueryResult sqr;
	sqr.soldier = mData->getCurrentSoldierID();
	sqr.team = mData->getCurrentTeamID();
	return sqr;
}

Common::QueryResult World::operator()(const Common::MovementInput& i)
{
	/* TODO: check client */
	if(!mData->movementAllowed(i)) {
		return DeniedQueryResult();
	}

	bool empty = (*mData)(i);
	assert(!empty);

	for(auto& q : mEventQueue) {
		q.push(InputEvent(i));
	}
	return InvalidQueryResult();
}

Common::QueryResult World::operator()(const Common::ShotInput& i)
{
	/* TODO */
	return DeniedQueryResult();
}

Common::QueryResult World::operator()(const Common::FinishTurnInput& i)
{
	/* TODO: check client */
	(*mData)(i);

	for(auto& q : mEventQueue) {
		q.push(InputEvent(i));
	}
	return InvalidQueryResult();
}

}

}

