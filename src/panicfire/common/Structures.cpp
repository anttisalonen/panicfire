#include <string.h>

#include <stdexcept>

#include "common/Random.h"

#include "panicfire/common/Structures.h"

#define SHOT_APS_REQUIRED	8

namespace PanicFire {

namespace Common {

bool SoldierData::alive() const
{
	return id.id != 0 && health.value > 0;
}

float Position::distance(const Position& oth) const
{
	int xd = oth.x - x;
	int yd = oth.y - y;
	return sqrt(xd * xd + yd * yd);
}

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
	auto& f = getPoint(p.x, p.y);
	return MapData::movementCost(f.grasslevel);
}

bool MapData::positionBlocked(const Position& p) const
{
	auto& f = getPoint(p.x, p.y);
	return f.wall || f.vegetationlevel != VegetationLevel::None;
}

WorldData::WorldData()
{
	for(auto &s : mCurrentSoldierIDIndex)
		s = 0;
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
				mSoldierData[sid - 1].position.x = w - j - 1;
			mSoldierData[sid - 1].position.y = i == 0 ? 0 : h - 1;
			mSoldierData[sid - 1].health.value = MAX_HEALTH;
			mSoldierData[sid - 1].direction = i == 0 ? Direction::SE : Direction::NW;
			mSoldierData[sid - 1].aps.value = MAX_APS;
			mTeamData[i].soldiers[j].id = sid;

			sid++;
		}
	}

	generateSoldierPositions();

	mCurrentTeamID = 1;
	for(auto &s : mCurrentSoldierIDIndex)
		s = 0;
}

bool WorldData::sync(WorldInterface& wi)
{
	{
		Common::QueryResult qr = wi.query(Common::MapQuery());
		if(!boost::apply_visitor(*this, qr)) {
			std::cerr << "Map query failed.\n";
			return false;
		}
		assert(this->getMapData());
	}

	for(unsigned int i = 1; i <= MAX_NUM_TEAMS; i++) {
		TeamID tid = TeamID(i);
		Common::QueryResult qr = wi.query(Common::TeamQuery(tid));
		if(!boost::apply_visitor(*this, qr)) {
			std::cerr << "Team query failed for team " << tid.id << ".\n";
			return false;
		}
		TeamData* td = getTeam(tid);
		assert(td);
		if(td) {
			for(unsigned int j = 0; j < MAX_TEAM_SOLDIERS; j++) {
				if(!td->soldiers[j].id)
					continue;
				SoldierID sid = td->soldiers[j];
				Common::QueryResult qr2 = wi.query(Common::SoldierQuery(sid));
				if(!boost::apply_visitor(*this, qr2)) {
					std::cerr << "Soldier query failed for soldier " << sid.id << ".\n";
					return false;
				}
			}
		}
	}

	Common::QueryResult qr = wi.query(Common::CurrentSoldierQuery());
	if(!boost::apply_visitor(*this, qr)) {
		std::cerr << "Current soldier query failed.\n";
		return false;
	}

	return true;
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

SoldierData& WorldData::getCurrentSoldier()
{
	auto td = getTeam(mCurrentTeamID);
	assert(td);
	auto sindex = soldierIndexFromSoldierID(td->soldiers[mCurrentSoldierIDIndex[teamIndexFromTeamID(mCurrentTeamID)]]);
	assert(sindex < mSoldierData.size());
	return mSoldierData[sindex];
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

const SoldierData& WorldData::getCurrentSoldier() const
{
	auto td = getTeam(mCurrentTeamID);
	assert(td);
	unsigned int sindex = soldierIndexFromSoldierID(td->soldiers[mCurrentSoldierIDIndex[teamIndexFromTeamID(mCurrentTeamID)]]);
	assert(sindex < mSoldierData.size());
	return mSoldierData[sindex];
}

SoldierID WorldData::getCurrentSoldierID() const
{
	return getCurrentSoldier().id;
}

TeamID WorldData::getCurrentTeamID() const
{
	return mCurrentTeamID;
}

void WorldData::advanceCurrent()
{
	// advance team ID
	mCurrentTeamID.id++;
	if(teamIndexFromTeamID(mCurrentTeamID) >= MAX_NUM_TEAMS)
		mCurrentTeamID.id = 1;
	auto tindex = teamIndexFromTeamID(mCurrentTeamID);

	assert(tindex < mCurrentSoldierIDIndex.size());

	// advance soldier ID within team
	int increments = 0;
	bool found = true;
	do {
		if(increments == MAX_TEAM_SOLDIERS) {
			std::cerr << "Warning: no live soldier found for team.\n";
			found = false;
			break;
		}
		mCurrentSoldierIDIndex[tindex]++;
		increments++;
		if(mCurrentSoldierIDIndex[tindex] >= MAX_TEAM_SOLDIERS) {
			mCurrentSoldierIDIndex[tindex] = 0;
		}
	} while(getCurrentSoldier().health.value == 0);

	if(found) {
		getCurrentSoldier().aps.value = MAX_APS;
	}

	assert(mCurrentSoldierIDIndex[tindex] < MAX_TEAM_SOLDIERS);
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
	mCurrentTeamID = q.team;
	unsigned int i = 0;
	bool found = false;
	auto td = getTeam(mCurrentTeamID);
	assert(td);
	for(auto& s : td->soldiers) {
		if(s == q.soldier) {
			mCurrentSoldierIDIndex[teamIndexFromTeamID(mCurrentTeamID)] = i;
			found = true;
			break;
		}
		i++;
	}
	if(found) {
		return true;
	} else {
		std::cerr << "Error: couldn't find referenced soldier in current soldier query.\n";
		return false;
	}
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
	sd->position = ev.to;
	sd->aps.value -= mMapData.movementCost(ev.to);
	sd->direction = getDirection(ev.from, ev.to);
	return false;
}

bool WorldData::operator()(const Common::ShotInput& ev)
{
	auto sd = getSoldier(ev.shooter);
	assert(sd);
	sd->aps -= APs(SHOT_APS_REQUIRED);
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

bool WorldData::operator()(const Common::SoldierWoundedEvent& ev)
{
	auto sd = getSoldier(ev.wounded);
	if(!sd) {
		std::cerr << "Warning: soldier wounded event received but no information on soldier.\n";
		return false;
	}

	sd->health = ev.newhealth;

	return false;
}

bool WorldData::operator()(const Common::GameWonEvent& ev)
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

	if(i.mover != getCurrentSoldierID()) {
		return false;
	}

	if(sd->position != i.from) {
		return false;
	}

	if(sd->position == i.to) {
		return false;
	}

	if(sd->aps.value < mMapData.movementCost(i.to)) {
		return false;
	}

	if(mMapData.positionBlocked(i.to)) {
		return false;
	}

	if(!(abs(sd->position.x - i.to.x) <= 1 && abs(sd->position.y - i.to.y) <= 1)) {
		return false;
	}

	if(getSoldierAt(i.to)) {
		return false;
	}

	return true;
}

bool WorldData::shotAllowed(const ShotInput& i) const
{
	auto sd = getSoldier(i.shooter);
	if(!sd) {
		return false;
	}

	if(i.shooter != getCurrentSoldierID()) {
		return false;
	}

	if(sd->aps.value < SHOT_APS_REQUIRED) {
		return false;
	}

	if(sd->position == i.target) {
		return false;
	}

	return true;
}

SoldierData* WorldData::getSoldierAt(const Position& p)
{
	for(auto& s : mSoldierData) {
		if(s.position == p && s.health.value > 0)
			return &s;
	}
	return nullptr;
}

const SoldierData* WorldData::getSoldierAt(const Position& p) const
{
	for(auto& s : mSoldierData) {
		if(s.position == p && s.health.value > 0)
			return &s;
	}
	return nullptr;
}

bool WorldData::teamLost(TeamID tid) const
{
	auto td = getTeam(tid);
	assert(td);
	for(auto sid : td->soldiers) {
		auto sd = getSoldier(sid);
		assert(sd);
		if(!sd) {
			std::cerr << "WorldData::teamLost called without full information\n";
			return false;
		}
		if(sd->health.value != 0)
			return false;
	}

	return true;
}

std::set<Position> WorldData::getSoldierPositions() const
{
	std::set<Position> ret;
	for(auto& sd : mSoldierData) {
		if(sd.health.value > 0)
			ret.insert(sd.position);
	}
	return ret;
}

void WorldData::syncCurrentSoldier(WorldInterface& wi)
{
	Common::QueryResult qr = wi.query(Common::CurrentSoldierQuery());
	if(!boost::apply_visitor(*this, qr)) {
		assert(0);
		throw std::runtime_error("Current soldier query failed");
	}
	Common::QueryResult qr2 = wi.query(Common::SoldierQuery(getCurrentSoldierID()));
	if(!boost::apply_visitor(*this, qr2)) {
		assert(0);
		throw std::runtime_error("Soldier query failed when syncing current");
	}
}

Direction WorldData::getDirection(const Position& from, const Position& to)
{
	assert(from != to);
	assert(abs(from.x - to.x) <= 1 && abs(from.y - to.y) <= 1);
	if(to.x > from.x) {
		if(to.y > from.y)
			return Direction::SE;
		else if(to.y == from.y)
			return Direction::E;
		else
			return Direction::NE;
	} else if(to.x == from.x) {
		if(to.y > from.y)
			return Direction::S;
		else
			return Direction::N;
	} else {
		if(to.y > from.y)
			return Direction::SW;
		else if(to.y == from.y)
			return Direction::W;
		else
			return Direction::NW;
	}
}

void WorldData::generateSoldierPositions()
{
	for(unsigned int i = 0; i < MAX_NUM_TEAMS; i++) {
		std::array<Position, MAX_TEAM_SOLDIERS> positions;
		for(unsigned int j = 0; j < MAX_TEAM_SOLDIERS; j++) {
			auto sd = getSoldier(mTeamData[i].soldiers[j]);
			assert(sd);
			Position p;
			int tries = 0;
			while(1) {
				tries++;
				if(tries > 100) {
					throw std::runtime_error("Unable to find position for soldier");
				}
				p.x = ::Common::Random::uniform(0, mMapData.getWidth());
				p.y = ::Common::Random::uniform(0, mMapData.getHeight());
				if(mMapData.positionBlocked(p)) {
					std::cout << p << " blocked\n";
					continue;
				}
				bool alreadyused = false;

				for(unsigned int l = 0; l < j; l++) {
					if(p.distance(positions[l]) < 5.0f) {
						alreadyused = true;
						std::cout << p << " used in own team\n";
						break;
					}
				}
				if(alreadyused)
					continue;

				for(unsigned int k = 0; k < i; k++) {
					for(unsigned int l = 0; l < MAX_TEAM_SOLDIERS; l++) {
						auto sd2 = getSoldier(mTeamData[k].soldiers[l]);
						assert(sd2);
						if(p.distance(sd2->position) < 5.0f) {
							alreadyused = true;
							std::cout << p << " used in opponent team\n";
							break;
						}
					}
				}
				if(alreadyused)
					continue;
				break;
			}
			sd->position = p;
			positions[j] = p;
			std::cout << sd->id << " at " << p << "\n";
		}
	}
}

}

}

