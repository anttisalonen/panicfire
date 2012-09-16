#ifndef PANICFIRE_COMMON_STRUCTURES_H
#define PANICFIRE_COMMON_STRUCTURES_H

#include <vector>

#include <boost/variant.hpp>

namespace PanicFire {

namespace Common {

struct SoldierID {
	int id = 0;
};

struct Position {
	int x = 0;
	int y = 0;
};

struct MovementEvent {
	SoldierID soldier;
	Position from;
	Position to;
};

struct Health {
	int health = 0;
};

struct SoldierData {
	SoldierID id;
	Position position;
	Health health;
};

enum class Vegetation {
	None,
	Tree1,
	Tree2,
	Tree3,
	Bush,
	Rock
};

enum class GrassLevel {
	Low,
	Medium,
	High
};

struct MapFragment {
	bool wall = false;
	bool floor = false;
	Vegetation vegetationlevel = Vegetation::None;
	GrassLevel grasslevel = GrassLevel::Low;
};

class MapData {
	public:
		const MapFragment& getPoint(int x, int y) const;
		int getWidth() const;
		int getHeight() const;

	private:
		int width = 0;
		int height = 0;
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

typedef boost::variant<SoldierQuery, MapQuery> Query;

// query results
struct SoldierQueryResult {
	SoldierData soldier;
};

struct MapQueryResult {
	MapData map;
};

struct DeniedQueryResult {
};

typedef boost::variant<SoldierQueryResult, MapQueryResult, DeniedQueryResult> QueryResult;

// interface
class GameInterface {
	public:
		virtual ~GameInterface() { }
		virtual QueryResult query(const Query& q) = 0;
		virtual bool input(const Input& i) = 0;
		virtual Event pollEvents() = 0;
};

}

}

#endif

