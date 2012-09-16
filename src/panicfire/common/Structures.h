#ifndef PANICFIRE_COMMON_STRUCTURES_H
#define PANICFIRE_COMMON_STRUCTURES_H

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

struct SightingEvent {
	SoldierID seer;
	SoldierID seen;
};

struct ShotEvent {
	SoldierID shooter;
	Position from;
	Position to;
	SoldierID hit;
};

typedef boost::variant<MovementEvent, SightingEvent, ShotEvent> Event;

}

}

#endif

