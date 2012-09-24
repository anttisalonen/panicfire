#include <iostream>

#include "panicfire/game/World.h"

namespace PanicFire { 

namespace Game {

using namespace PanicFire::Common;

World::World()
{
	mData = new WorldData(24, 24, MAX_TEAM_SOLDIERS);
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
	/* TODO: check client */
	if(!mData->shotAllowed(i)) {
		return DeniedQueryResult();
	}

	bool empty = (*mData)(i);
	assert(!empty);

	/* TODO: add checking for obstacles and range */
	for(auto& q : mEventQueue) {
		q.push(InputEvent(i));
	}

	auto tgtsoldier = mData->getSoldierAt(i.target);
	if(tgtsoldier) {
		Health nh(tgtsoldier->health);
		nh -= Health(40);
		SoldierWoundedEvent ev(tgtsoldier->id, nh);

		bool empty = (*mData)(ev);
		assert(!empty);

		for(auto& q : mEventQueue) {
			q.push(ev);
		}

		if(nh.value == 0) {
			TeamID t = tgtsoldier->teamid;
			if(mData->teamLost(t)) {
				static_assert(MAX_NUM_TEAMS == 2, "currently only two teams are supported");
				GameWonEvent gwe(t.id == 1 ? TeamID(2) : TeamID(1));

				bool empty = (*mData)(gwe);
				assert(!empty);

				for(auto& q : mEventQueue) {
					q.push(gwe);
				}

			}
		}
	}
	return InvalidQueryResult();
}

Common::QueryResult World::operator()(const Common::FinishTurnInput& i)
{
	/* TODO: check client */
	(*mData)(i);

	mData->advanceCurrent();

	for(auto& q : mEventQueue) {
		q.push(InputEvent(i));
	}
	return InvalidQueryResult();
}

}

}

