#ifndef PANICFIRE_COMMON_STRUCTURES_H
#define PANICFIRE_COMMON_STRUCTURES_H

#include <iostream>
#include <vector>
#include <array>
#include <set>

#include <boost/variant.hpp>

#include "common/Math.h"

namespace PanicFire {

namespace Common {

#define MAX_TEAM_SOLDIERS	4
#define MAX_NUM_TEAMS		2

#define MAX_HEALTH	100
#define MAX_APS		25

struct SoldierID {
	SoldierID(unsigned int tid = 0) : id(tid) { }
	unsigned int id;
	bool operator<(const SoldierID& oth) const;
	bool operator==(const SoldierID& oth) const;
	bool operator!=(const SoldierID& oth) const;
};

inline std::ostream& operator<<(std::ostream& out, const SoldierID& p)
{
	out << "Soldier ID " << p.id;
	return out;
}

inline bool SoldierID::operator<(const SoldierID& oth) const
{
	return id < oth.id;
}

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

inline std::ostream& operator<<(std::ostream& out, const TeamID& p)
{
	out << "Team ID " << p.id;
	return out;
}

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

template<unsigned int maxx>
struct Capped {
	Capped(unsigned int h) : value(h) { value = ::Common::clamp(0u, value, maxx); }
	unsigned int value;
	inline Capped operator-(const Capped& rhs) const;
	inline void operator-=(const Capped& rhs);
	inline Capped operator+(const Capped& rhs) const;
	inline void operator+=(const Capped& rhs);
};

template<unsigned int maxx>
Capped<maxx> Capped<maxx>::operator-(const Capped& rhs) const
{
	Capped r(*this);
	r -= rhs;
	return r;
}

template<unsigned int maxx>
void Capped<maxx>::operator-=(const Capped& rhs)
{
	if(rhs.value > value)
		value = 0;
	else
		value -= rhs.value;
}

template<unsigned int maxx>
Capped<maxx> Capped<maxx>::operator+(const Capped& rhs) const
{
	Capped r(*this);
	r += rhs;
	return r;
}

template<unsigned int maxx>
void Capped<maxx>::operator+=(const Capped& rhs)
{
	value += rhs.value;
	if(value > maxx)
		value = maxx;
}

struct Health : public Capped<MAX_HEALTH> {
	Health(unsigned int h = MAX_HEALTH) : Capped(h) { }
};

struct APs : public Capped<MAX_APS> {
	APs(unsigned int h = MAX_APS) : Capped(h) { }
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
	APs aps;
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
		unsigned int movementCost(const Position& p) const;

	private:
		unsigned int width = 0;
		unsigned int height = 0;
		std::vector<MapFragment> data;
};

// input
struct MovementInput {
	MovementInput(SoldierID i, const Position& fr, const Position& p) : mover(i), from(fr), to(p) { }
	SoldierID mover;
	Position from;
	Position to;
};

struct ShotInput {
	ShotInput(SoldierID i, const Position& p) : shooter(i), target(p) { }
	SoldierID shooter;
	Position target;
};

struct FinishTurnInput {
};

typedef boost::variant<MovementInput, ShotInput, FinishTurnInput> Input;

// events
struct SightingEvent {
	SoldierID seer;
	SoldierID seen;
};

struct SoldierWoundedEvent {
	SoldierWoundedEvent(SoldierID w, Health h) : wounded(w), newhealth(h) { }
	SoldierID wounded;
	Health newhealth;
};

struct GameWonEvent {
	GameWonEvent(TeamID w) : winner(w) { }
	TeamID winner;
};

struct EmptyEvent {
};

struct InputEvent {
	InputEvent(const Input& i) : input(i) { }
	Input input;
};

typedef boost::variant<InputEvent, SightingEvent, SoldierWoundedEvent, GameWonEvent, EmptyEvent> Event;

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
	TeamID team;
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
		virtual Event pollEvents(TeamID tid) = 0;
};

class WorldData : public boost::static_visitor<bool> {
	public:
		WorldData();
		WorldData(unsigned int w, unsigned int h, unsigned int nsoldiers);

		static TeamID teamIDFromSoldierID(SoldierID s);
		bool sync(WorldInterface& wi);

		TeamData* getTeam(TeamID t);
		TeamData* getTeam(SoldierID t);
		SoldierData* getSoldier(SoldierID s);
		MapData* getMapData();
		SoldierData& getCurrentSoldier();
		SoldierData* getSoldierAt(const Position& p);

		const TeamData* getTeam(TeamID t) const;
		const TeamData* getTeam(SoldierID t) const;
		const SoldierData* getSoldier(SoldierID s) const;
		const MapData* getMapData() const;
		const SoldierData& getCurrentSoldier() const;
		const SoldierData* getSoldierAt(const Position& p) const;

		SoldierID getCurrentSoldierID() const;
		TeamID getCurrentTeamID() const;
		void advanceCurrent();
		bool movementAllowed(const MovementInput& i) const;
		bool shotAllowed(const ShotInput& i) const;

		// call this function only for a team where all the soldiers are known
		bool teamLost(TeamID tid) const;

		std::set<Position> getSoldierPositions() const;
		void syncCurrentSoldier(WorldInterface& wi);

		bool operator()(const Common::SoldierQueryResult& q);
		bool operator()(const Common::TeamQueryResult& q);
		bool operator()(const Common::MapQueryResult& q);
		bool operator()(const Common::CurrentSoldierQueryResult& q);
		bool operator()(const Common::DeniedQueryResult& q);
		bool operator()(const Common::InvalidQueryResult& q);

		// event handling - return true for EmptyEvent only
		bool operator()(const Common::InputEvent& ev);
		bool operator()(const Common::SightingEvent& ev);
		bool operator()(const Common::SoldierWoundedEvent& ev);
		bool operator()(const Common::GameWonEvent& ev);
		bool operator()(const Common::EmptyEvent& ev);

		// input event handling
		bool operator()(const Common::MovementInput& ev);
		bool operator()(const Common::ShotInput& ev);
		bool operator()(const Common::FinishTurnInput& ev);

		static unsigned int teamIndexFromTeamID(TeamID s);
		static unsigned int soldierIndexFromSoldierID(SoldierID s);

	private:
		static Direction getDirection(const Position& from, const Position& to);
		MapData mMapData;
		std::array<TeamData, MAX_NUM_TEAMS> mTeamData;
		std::array<SoldierData, MAX_NUM_TEAMS * MAX_TEAM_SOLDIERS> mSoldierData;
		TeamID mCurrentTeamID;
		std::array<unsigned int, MAX_NUM_TEAMS> mCurrentSoldierIDIndex;
};

}

}

#endif

