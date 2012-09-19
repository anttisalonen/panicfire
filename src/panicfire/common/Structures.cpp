#include <stdexcept>

#include "common/Random.h"

#include "panicfire/common/Structures.h"

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
			mTeamData[i].soldiers[j].id = sid;

			sid++;
		}
	}
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


}

}

