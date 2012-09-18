#ifndef PANICFIRE_COMMON_STRUCTURES_H
#define PANICFIRE_COMMON_STRUCTURES_H

#include <vector>

#include <boost/variant.hpp>

namespace PanicFire {

namespace Common {

#define MAX_TEAM_SOLDIERS 8
#define MAX_NUM_TEAMS 2

struct SoldierID {
	unsigned int id = 0;
};

struct TeamID {
	unsigned int id = 0;
};

struct Position {
	unsigned int x = 0;
	unsigned int y = 0;
};

struct MovementEvent {
	SoldierID soldier;
	Position from;
	Position to;
};

struct Health {
	unsigned int health = 0;
};

enum class Direction {
	E,
	NE,
	N,
	NW,
	W,
	SW,
	S,
	SE
};

struct SoldierData {
	SoldierID id;
	TeamID teamid;
	Position position;
	Health health;
	bool active = false;
	Direction direction;
};

struct TeamData {
	TeamID id;
	SoldierID soldiers[MAX_TEAM_SOLDIERS];
};

enum class VegetationLevel {
	None,
	Tree1,
	Tree2,
	Tree3,
	Bush,
	Rock
};

enum class GrassLevel {
	Floor,
	Path,
	Low,
	Medium,
	High
};

struct MapFragment {
	bool wall = false;
	VegetationLevel vegetationlevel = VegetationLevel::None;
	GrassLevel grasslevel = GrassLevel::Low;
};

class MapData {
	public:
		void generate(unsigned int w, unsigned int h);
		const MapFragment& getPoint(unsigned int x, unsigned int y) const;
		unsigned int getWidth() const;
		unsigned int getHeight() const;

	private:
		unsigned int width = 0;
		unsigned int height = 0;
		std::vector<MapFragment> data;
};

// events
struct SightingEvent {
	SoldierID seer;
	SoldierID seen;
};

struct ShotEvent {
	SoldierID shooter;
	SoldierID hit;
	Position from;
	Position to;
};

struct EmptyEvent {
};

typedef boost::variant<MovementEvent, SightingEvent, ShotEvent, EmptyEvent> Event;

// input
struct MovementInput {
	SoldierID mover;
	Position to;
};

struct ShotInput {
	SoldierID shooter;
	Position target;
};

struct FinishTurnInput {
};

typedef boost::variant<MovementInput, ShotInput, FinishTurnInput> Input;

// queries
struct SoldierQuery {
	SoldierID soldier;
};

struct MapQuery {
};

struct TeamQuery {
	TeamID team;
};

typedef boost::variant<SoldierQuery, MapQuery, TeamQuery> Query;

// query results
struct SoldierQueryResult {
	SoldierData soldier;
};

struct MapQueryResult {
	MapData map;
};

struct TeamQueryResult {
	TeamData team;
};

struct InvalidQueryResult {
};

struct DeniedQueryResult {
};

typedef boost::variant<SoldierQueryResult, MapQueryResult, TeamQueryResult, InvalidQueryResult, DeniedQueryResult> QueryResult;

// interface
class WorldInterface {
	public:
		virtual ~WorldInterface() { }
		virtual QueryResult query(const Query& q) = 0;
		virtual bool input(const Input& i) = 0;
		virtual Event pollEvents() = 0;
};

class WorldData {
	public:
		WorldData(unsigned int w, unsigned int h, unsigned int nsoldiers);

		static TeamID teamIDFromSoldierID(SoldierID s);
		TeamData* getTeam(TeamID t);
		TeamData* getTeam(SoldierID t);
		SoldierData* getSoldier(SoldierID s);
		MapData* getMapData();

	private:
		static unsigned int teamIndexFromTeamID(TeamID s);
		static unsigned int soldierIndexFromSoldierID(SoldierID s);
		MapData mMapData;
		TeamData mTeamData[MAX_NUM_TEAMS];
		SoldierData mSoldierData[MAX_NUM_TEAMS * MAX_TEAM_SOLDIERS];
};

}

}

#endif

