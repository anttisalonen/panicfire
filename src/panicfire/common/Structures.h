#ifndef PANICFIRE_COMMON_STRUCTURES_H
#define PANICFIRE_COMMON_STRUCTURES_H

#include <iostream>
#include <vector>
#include <array>

#include <boost/variant.hpp>

namespace PanicFire {

namespace Common {

#define MAX_TEAM_SOLDIERS 8
#define MAX_NUM_TEAMS 2

struct SoldierID {
	SoldierID(unsigned int tid = 0) : id(tid) { }
	unsigned int id;
	bool operator==(const SoldierID& oth) const;
	bool operator!=(const SoldierID& oth) const;
};

inline bool SoldierID::operator==(const SoldierID& oth) const
{
	return id == oth.id;
}

inline bool SoldierID::operator!=(const SoldierID& oth) const
{
	return !(*this == oth);
}

struct TeamID {
	TeamID(unsigned int tid = 0) : id(tid) { }
	unsigned int id;
	bool operator==(const TeamID& oth) const;
	bool operator!=(const TeamID& oth) const;
};

inline bool TeamID::operator==(const TeamID& oth) const
{
	return id == oth.id;
}

inline bool TeamID::operator!=(const TeamID& oth) const
{
	return !(*this == oth);
}

struct Position {
	Position(unsigned int x_ = 0, unsigned int y_ = 0) : x(x_), y(y_) { } 
	unsigned int x;
	unsigned int y;
	bool operator<(const Position& oth) const;
	bool operator==(const Position& oth) const;
	bool operator!=(const Position& oth) const;
};

inline std::ostream& operator<<(std::ostream& out, const Position& p)
{
	out << "(" << p.x << ", " << p.y << ")";
	return out;
}

inline bool Position::operator<(const Position& oth) const
{
	if(x < oth.x)
		return true;
	if(x > oth.x)
		return false;
	return y < oth.y;
}

inline bool Position::operator==(const Position& oth) const
{
	return x == oth.x && y == oth.y;
}

inline bool Position::operator!=(const Position& oth) const
{
	return !(*this == oth);
}

struct MovementEvent {
	MovementEvent(SoldierID s, const Position& f, const Position& t)
		: soldier(s), from(f), to(t) { }
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
		static unsigned int movementCost(GrassLevel g);

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
	MovementInput(SoldierID i, const Position& p) : mover(i), to(p) { }
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
	SoldierQuery(SoldierID tid) : soldier(tid) { }
	SoldierID soldier;
};

struct MapQuery {
};

struct TeamQuery {
	TeamQuery(TeamID tid) : team(tid) { }
	TeamID team;
};

struct CurrentSoldierQuery {
};

typedef boost::variant<SoldierQuery, MapQuery, TeamQuery, CurrentSoldierQuery> Query;

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

struct CurrentSoldierQueryResult {
	SoldierID soldier;
};

struct InvalidQueryResult {
};

struct DeniedQueryResult {
};

typedef boost::variant<SoldierQueryResult, MapQueryResult, TeamQueryResult,
	CurrentSoldierQueryResult, InvalidQueryResult, DeniedQueryResult> QueryResult;

// interface
class WorldInterface {
	public:
		virtual ~WorldInterface() { }
		virtual QueryResult query(const Query& q) = 0;
		virtual bool input(const Input& i) = 0;
		virtual Event pollEvents() = 0;
};

class WorldData : public boost::static_visitor<bool> {
	public:
		WorldData();
		WorldData(unsigned int w, unsigned int h, unsigned int nsoldiers);

		static TeamID teamIDFromSoldierID(SoldierID s);

		TeamData* getTeam(TeamID t);
		TeamData* getTeam(SoldierID t);
		SoldierData* getSoldier(SoldierID s);
		MapData* getMapData();
		SoldierData* getCurrentSoldier();

		const TeamData* getTeam(TeamID t) const;
		const TeamData* getTeam(SoldierID t) const;
		const SoldierData* getSoldier(SoldierID s) const;
		const MapData* getMapData() const;
		const SoldierData* getCurrentSoldier() const;

		SoldierID getCurrentSoldierID() const;
		void setCurrentSoldierID(SoldierID i);
		bool movementAllowed(const MovementInput& i) const;

		bool operator()(const Common::SoldierQueryResult& q);
		bool operator()(const Common::TeamQueryResult& q);
		bool operator()(const Common::MapQueryResult& q);
		bool operator()(const Common::CurrentSoldierQueryResult& q);
		bool operator()(const Common::DeniedQueryResult& q);
		bool operator()(const Common::InvalidQueryResult& q);

		bool operator()(const Common::MovementEvent& ev);
		bool operator()(const Common::SightingEvent& ev);
		bool operator()(const Common::ShotEvent& ev);
		bool operator()(const Common::EmptyEvent& ev);

		static unsigned int teamIndexFromTeamID(TeamID s);
		static unsigned int soldierIndexFromSoldierID(SoldierID s);

	private:
		MapData mMapData;
		std::array<TeamData, MAX_NUM_TEAMS> mTeamData;
		std::array<SoldierData, MAX_NUM_TEAMS * MAX_TEAM_SOLDIERS> mSoldierData;
		SoldierID mCurrentSoldierID;
};

}

}

#endif

