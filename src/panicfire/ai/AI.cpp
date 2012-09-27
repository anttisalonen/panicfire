#include <iostream>
#include <stdexcept>

#include "common/Rectangle.h"
#include "common/SDL_utils.h"
#include "common/Random.h"
#include "common/Line.h"

#include "panicfire/ai/AI.h"

using namespace Common;
using namespace PanicFire;
using namespace PanicFire::Common;

namespace PanicFire {

namespace AI {

// AIData
AIData::AIData(Common::WorldInterface& w)
	: mWorld(w),
	mMyTeamID(TeamID(2)),
	mGameOver(false),
	mMyTurn(false)
{
	if(!mData.sync(mWorld))
		throw std::runtime_error("Fail on sync data");

	mTeamPlan.setAIData(this);

	mAStar.setMapData(mData.getMapData());
}

void AIData::updateCurrentSoldier()
{
	mData.syncCurrentSoldier(mWorld);
	mMyTurn = mData.getCurrentTeamID() == mMyTeamID;
}

// TeamPlan
TeamPlan::TeamPlan()
	: mAIData(nullptr)
{
}

void TeamPlan::setAIData(AIData* d)
{
	mAIData = d;
}

void TeamPlan::positionVisited(const Position& p)
{
	mVisitPositions.erase(p);
}

Position TeamPlan::getNextVisitPosition() const
{
	assert(mAIData);
	if(mVisitPositions.empty()) {
		auto w = mAIData->mData.getMapData()->getWidth();
		auto h = mAIData->mData.getMapData()->getHeight();
		if(w == 0 || h == 0) {
			throw std::runtime_error("AI: empty map");
		}
		unsigned int numPositions = w * h / 20;
		for(unsigned int i = 0; i < numPositions; i++) {
			mVisitPositions.insert(Position(Random::uniform(0, w),
						Random::uniform(0, h)));
		}
	}

	assert(!mVisitPositions.empty());
	unsigned int mmax = mVisitPositions.size();
	unsigned int t = Random::uniform(0, mmax);
	for(std::set<Position>::const_iterator it = mVisitPositions.begin(); it != mVisitPositions.end(); ++it) {
		if(t == 0) {
			Position p = *it;
			mVisitPositions.erase(it);
			return p;
		}
		else
			t--;
	}
	assert(0);
	return *mVisitPositions.begin();
}

// SoldierPlan
SoldierPlan::SoldierPlan(AIData& d, SoldierID i)
	: mAIData(d),
	mID(i),
	mSentInput(false),
	mShooting(false)
{
	mTargetPosition = mAIData.mData.getSoldier(mID)->position;
}

void SoldierPlan::act()
{
	do {
		handleEvents();
		if(mAIData.mMyTurn) {
			checkShotChance();
			sendInput();
		}
	} while(mAIData.mMyTurn);
}

void SoldierPlan::setupPath()
{
	auto sd = mAIData.mData.getSoldier(mID);
	if(sd->position == mTargetPosition) {
		do {
			mTargetPosition = mAIData.mTeamPlan.getNextVisitPosition();
			mPath = mAIData.mAStar.solve(mAIData.mData.getSoldierPositions(),
					sd->position, mTargetPosition);
		} while(mPath.empty());
	}
}

void SoldierPlan::sendInput()
{
	auto sd = mAIData.mData.getSoldier(mID);
	if(mShooting) {
		bool succ = mAIData.mWorld.input(ShotInput(mID, mShootPosition));
		if(!succ) {
			bool succ = mAIData.mWorld.input(FinishTurnInput());
			assert(succ);
		} else {
			mSentInput = true;
		}
	} else {
		if(mPath.empty() || *mPath.begin() == sd->position) {
			setupPath();
			assert(!mPath.empty());
		}

		for(auto pit = mPath.begin(); pit != mPath.end(); ) {
			if(sd->position == *pit) {
				pit = mPath.erase(pit);
			} else {
				MovementInput i(mID, sd->position, *pit);
				if(mAIData.mData.movementAllowed(i)) {
					bool succ = mAIData.mWorld.input(i);
					assert(succ);
					mSentInput = true;
				} else {
					bool succ = mAIData.mWorld.input(FinishTurnInput());
					assert(succ);
				}
				break;
			}
		}
	}
}

void SoldierPlan::handleEvents()
{
	while(1) {
		auto ev = mAIData.mWorld.pollEvents(mAIData.mMyTeamID);
		bool empty = boost::apply_visitor(mAIData.mData, ev);
		if(empty)
			break;
		boost::apply_visitor(*this, ev);
	}
}

void SoldierPlan::checkShotChance()
{
	auto mysd = mAIData.mData.getSoldier(mID);
	mShooting = false;

	static_assert(MAX_NUM_TEAMS == 2, "Only two teams supported");
	TeamID other = mAIData.mMyTeamID == TeamID(1) ? TeamID(2) : TeamID(1);
	auto td = mAIData.mData.getTeam(other);
	assert(td);
	for(auto sid : td->soldiers) {
		auto sd = mAIData.mData.getSoldier(sid);
		if(sd && sd->alive()) {
			auto l = Line::line(Point2(mysd->position.x, mysd->position.y),
					Point2(sd->position.x, sd->position.y));
			assert(l.size() >= 2);
			l.pop_front(); // shooter position
			l.pop_front(); // first position next to shooter
			if(!l.empty())
				l.pop_back(); // target location
			const auto mapdata = mAIData.mData.getMapData();
			bool blocked = false;
			for(auto& p : l) {
				Position pp(p.x, p.y);
				if(mapdata->positionBlocked(pp) || mAIData.mData.getSoldierAt(pp)) {
					blocked = true;
					break;
				}
			}
			if(!blocked) {
				mShootPosition = sd->position;
				mShooting = true;
				break;
			}
		}
	}
}

void SoldierPlan::operator()(const Common::InputEvent& ev)
{
	boost::apply_visitor(*this, ev.input);
}

void SoldierPlan::operator()(const Common::SightingEvent& ev)
{
}

void SoldierPlan::operator()(const Common::SoldierWoundedEvent& ev)
{
}

void SoldierPlan::operator()(const Common::GameWonEvent& ev)
{
	mAIData.mGameOver = true;
}

void SoldierPlan::operator()(const Common::EmptyEvent& ev)
{
	assert(0);
}

void SoldierPlan::operator()(const Common::MovementInput& ev)
{
	if(mSentInput &&
			ev.mover == mID) {
		mSentInput = false;
	}
}

void SoldierPlan::operator()(const Common::ShotInput& ev)
{
	if(mSentInput &&
			ev.shooter == mID) {
		mSentInput = false;
	}
}

void SoldierPlan::operator()(const Common::FinishTurnInput& ev)
{
	mAIData.updateCurrentSoldier();
}

AI::AI(Common::WorldInterface& w)
	: mAIData(w)
{
}

AI::~AI()
{
}

void AI::act()
{
	mAIData.updateCurrentSoldier();

	if(mAIData.mGameOver)
		return;

	sendInput();
}

void AI::sendInput()
{
	auto sd = mAIData.mData.getCurrentSoldier();
	assert(sd.teamid == mAIData.mData.getCurrentTeamID());
	assert(sd.teamid == mAIData.mMyTeamID);

	auto it = mAIData.mSoldierPlan.find(sd.id);
	if(it == mAIData.mSoldierPlan.end()) {
		it = mAIData.mSoldierPlan.insert({sd.id, SoldierPlan(mAIData, sd.id)}).first;
	}
	it->second.act();
}


}

}


