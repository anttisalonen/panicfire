#include <stdexcept>

#include "common/Random.h"

#include "panicfire/common/Structures.h"

#define MAX_APS	25

namespace PanicFire {

namespace Common {

void MapData::generate(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
	data.resize(width * height);
	for(unsigned int j = 0; j < width; j++) {
		for(unsigned int i = 0; i < height; i++) {
			int gl = ::Common::Random::uniform(2, 5);
			int v = ::Common::Random::uniform(0, 3);
			int r = ::Common::Random::uniform(1, 4);
			data[j * height + i].grasslevel = static_cast<GrassLevel>(gl);
			if(v == 0) {
				data[j * height + i].vegetationlevel = static_cast<VegetationLevel>(r);
			}
		}
	}
}

const MapFragment& MapData::getPoint(unsigned int x, unsigned int y) const
{
	if(x >= width || y >= height)
		throw std::runtime_error("MapData: access outside boundary");
	return data[y * height + x];
}

unsigned int MapData::getWidth() const
{
	return width;
}

unsigned int MapData::getHeight() const
{
	return height;
}

unsigned int MapData::movementCost(GrassLevel g)
{
	switch(g) {
		case GrassLevel::Floor:
		case GrassLevel::Path:
			return 2;
		case GrassLevel::Low:
			return 3;
		case GrassLevel::Medium:
			return 4;
		case GrassLevel::High:
			return 5;
	}
	assert(0);
	throw std::runtime_error("MapData::movementCost: unreachable");
}

unsigned int MapData::movementCost(const Position& p) const
{
	auto f = getPoint(p.x, p.y);
	return MapData::movementCost(f.grasslevel);
}

WorldData::WorldData()
{
}

WorldData::WorldData(unsigned int w, unsigned int h, unsigned int nsoldiers)
{
	mMapData.generate(w, h);
	nsoldiers = std::min(static_cast<unsigned int>(MAX_TEAM_SOLDIERS), nsoldiers);
	unsigned int sid = 1;
	for(unsigned int i = 0; i < MAX_NUM_TEAMS; i++) {
		static_assert(MAX_NUM_TEAMS == 2, "currently only two teams are supported");
		unsigned int tid = i + 1;
		mTeamData[i].id.id = tid;
		for(unsigned int j = 0; j < nsoldiers; j++) {
			mSoldierData[sid - 1].id.id = sid;
			mSoldierData[sid - 1].teamid.id = tid;
			if(i == 0)
				mSoldierData[sid - 1].position.x = j;
			else
				mSoldierData[sid - 1].position.x = w - j;
			mSoldierData[sid - 1].position.y = i == 0 ? 0 : h - 1;;
			mSoldierData[sid - 1].health.health = 100;
			mSoldierData[sid - 1].active = true;
			mSoldierData[sid - 1].direction = i == 0 ? Direction::SE : Direction::NW;
			mSoldierData[sid - 1].aps.aps = MAX_APS;
			mTeamData[i].soldiers[j].id = sid;

			sid++;
		}
	}
	mCurrentSoldierID = 1;
}

unsigned int WorldData::teamIndexFromTeamID(TeamID s)
{
	return s.id - 1;
}

unsigned int WorldData::soldierIndexFromSoldierID(SoldierID s)
{
	return s.id - 1;
}

TeamID WorldData::teamIDFromSoldierID(SoldierID s)
{
	TeamID t;
	t.id = (s.id - 1) / MAX_TEAM_SOLDIERS;
	t.id += 1;
	return t;
}

TeamData* WorldData::getTeam(TeamID t)
{
	unsigned int i = teamIndexFromTeamID(t);
	if(i < mTeamData.size())
		return &mTeamData[i];
	else
		return nullptr;
}

TeamData* WorldData::getTeam(SoldierID t)
{
	return getTeam(teamIDFromSoldierID(t));
}

SoldierData* WorldData::getSoldier(SoldierID s)
{
	unsigned int i = soldierIndexFromSoldierID(s);
	if(i < mSoldierData.size()) {
		return &mSoldierData[i];
	} else {
		return nullptr;
	}
}

MapData* WorldData::getMapData()
{
	return &mMapData;
}

SoldierData* WorldData::getCurrentSoldier()
{
	return getSoldier(mCurrentSoldierID);
}

const TeamData* WorldData::getTeam(TeamID t) const
{
	unsigned int i = teamIndexFromTeamID(t);
	if(i < mTeamData.size())
		return &mTeamData[i];
	else
		return nullptr;
}

const TeamData* WorldData::getTeam(SoldierID t) const
{
	return getTeam(teamIDFromSoldierID(t));
}

const SoldierData* WorldData::getSoldier(SoldierID s) const
{
	unsigned int i = soldierIndexFromSoldierID(s);
	if(i < mSoldierData.size()) {
		return &mSoldierData[i];
	} else {
		return nullptr;
	}
}

const MapData* WorldData::getMapData() const
{
	return &mMapData;
}

const SoldierData* WorldData::getCurrentSoldier() const
{
	return getSoldier(mCurrentSoldierID);
}

SoldierID WorldData::getCurrentSoldierID() const
{
	return mCurrentSoldierID;
}

void WorldData::setCurrentSoldierID(SoldierID i)
{
	mCurrentSoldierID = i;
}

bool WorldData::operator()(const Common::SoldierQueryResult& q)
{
	unsigned int index = soldierIndexFromSoldierID(q.soldier.id);
	if(index >= mSoldierData.size()) {
		std::cerr << "Soldier query failed (invalid data - ID: " << q.soldier.id.id << ").\n";
		assert(0);
		return false;
	}
	mSoldierData[index] = q.soldier;
	std::cout << "Soldier query successful.\n";
	return true;
}

bool WorldData::operator()(const Common::TeamQueryResult& q)
{
	unsigned int index = teamIndexFromTeamID(q.team.id);
	if(index >= mTeamData.size()) {
		assert(0);
		std::cerr << "Team query failed (invalid data).\n";
		return false;
	}
	mTeamData[index] = q.team;
	std::cout << "Team query successful for team " << mTeamData[index].id.id << ".\n";
	return true;
}

bool WorldData::operator()(const Common::MapQueryResult& q)
{
	std::cout << "Map query successful.\n";
	mMapData = q.map;
	return true;
}

bool WorldData::operator()(const Common::CurrentSoldierQueryResult& q)
{
	std::cout << "Current soldier query successful.\n";
	mCurrentSoldierID = q.soldier;
	return true;
}

bool WorldData::operator()(const Common::DeniedQueryResult& q)
{
	std::cerr << "WorldData error: denied query.\n";
	return false;
}

bool WorldData::operator()(const Common::InvalidQueryResult& q)
{
	std::cerr << "WorldData error: invalid query.\n";
	return false;
}

bool WorldData::operator()(const Common::MovementInput& ev)
{
	auto sd = getSoldier(ev.mover);
	assert(sd);
	assert(movementAllowed(ev));
	sd->position = ev.to;
	sd->aps.aps -= mMapData.movementCost(ev.to);
	return false;
}

bool WorldData::operator()(const Common::ShotInput& ev)
{
	return false;
}

bool WorldData::operator()(const Common::FinishTurnInput& ev)
{
	return false;
}

bool WorldData::operator()(const Common::InputEvent& ev)
{
	boost::apply_visitor(*this, ev.input);
	return false;
}

bool WorldData::operator()(const Common::SightingEvent& ev)
{
	return false;
}

bool WorldData::operator()(const Common::EmptyEvent& ev)
{
	return true;
}

bool WorldData::movementAllowed(const MovementInput& i) const
{
	auto sd = getSoldier(i.mover);
	if(!sd) {
		return false;
	}

	if(i.mover != mCurrentSoldierID) {
		return false;
	}

	if(sd->position == i.to) {
		return false;
	}

	if(sd->aps.aps < mMapData.movementCost(i.to)) {
		return false;
	}

	return abs(sd->position.x - i.to.x) <= 1 &&
			abs(sd->position.y - i.to.y) <= 1;
}

}

}

