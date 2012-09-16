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
class WorldInterface {
	public:
		virtual ~WorldInterface() { }
		virtual QueryResult query(const Query& q) = 0;
		virtual bool input(const Input& i) = 0;
		virtual Event pollEvents() = 0;
};

}

}

#endif

