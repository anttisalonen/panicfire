#include <iostream>
#include <stdexcept>

#include "common/Rectangle.h"
#include "common/SDL_utils.h"

#include "panicfire/ai/AI.h"

using namespace Common;
using namespace PanicFire;
using namespace PanicFire::Common;

namespace PanicFire {

namespace AI {

AI::AI(Common::WorldInterface& w)
	: mWorld(w),
	mMyTeamID(TeamID(2)),
	mMoving(false)
{
	if(!mData.sync(mWorld))
		throw std::runtime_error("Fail on sync data");

	mAStar.setMapData(mData.getMapData());
}

AI::~AI()
{
}


void AI::operator()(const Common::InputEvent& ev)
{
	boost::apply_visitor(*this, ev.input);
}

void AI::operator()(const Common::SightingEvent& ev)
{
}

void AI::operator()(const Common::EmptyEvent& ev)
{
	assert(0);
}


void AI::operator()(const Common::MovementInput& ev)
{
	if(mMoving &&
			mData.teamIDFromSoldierID(ev.mover) == mMyTeamID &&
			ev.mover == mCommandedSoldierID && ev.to == mMovementPosition) {
		mMoving = false;
	}
}

void AI::operator()(const Common::ShotInput& ev)
{
}

void AI::operator()(const Common::FinishTurnInput& ev)
{
	updateCurrentSoldier();
}


void AI::act()
{
	handleEvents();
	sendEndOfTurn();
}


void AI::sendInput()
{
	if(!mPathLine.empty() && !mMoving) {
		auto sd = mData.getCurrentSoldier();
		assert(sd);
		for(auto pit = mPathLine.begin(); pit != mPathLine.end(); ) {
			if(sd->position == *pit) {
				pit = mPathLine.erase(pit);
			} else {
				MovementInput i(mData.getCurrentSoldierID(), *pit);
				if(mData.movementAllowed(i)) {
					bool succ = mWorld.input(i);
					assert(succ);
					if(succ) {
						mMoving = true;
						mMovementPosition = *pit;
						mCommandedSoldierID = sd->id;
					}
				}
				break;
			}
		}
	}
}

void AI::sendEndOfTurn()
{
	if(mData.getCurrentTeamID() != mMyTeamID)
		return;

	bool succ = mWorld.input(FinishTurnInput());
	assert(succ);
	mPathLine.clear();
	mMoving = false;
}

void AI::handleEvents()
{
	while(1) {
		auto ev = mWorld.pollEvents(mMyTeamID);
		bool empty = boost::apply_visitor(mData, ev);
		if(empty)
			break;
		boost::apply_visitor(*this, ev);
	}
}

void AI::updateCurrentSoldier()
{
	Common::QueryResult qr = mWorld.query(Common::CurrentSoldierQuery());
	if(!boost::apply_visitor(mData, qr)) {
		std::cerr << "AI: current soldier query failed.\n";
		assert(0);
	}
}


}

}


