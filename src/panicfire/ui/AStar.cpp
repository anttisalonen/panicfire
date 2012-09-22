#include "panicfire/ui/AStar.h"

#include "common/AStar.h"

namespace PanicFire {

namespace UI {

using Common::Position;

AStar::AStar()
{
}

void AStar::setMapData(const Common::MapData* m)
{
	mMapData = m;
}

std::list<Common::Position> AStar::solve(const std::set<Common::Position>& blocked,
		const Common::Position& from,
		const Common::Position& to) const
{
	return ::Common::AStar<Common::Position>::solve([&](const Position& p) {
				return graphFunc(blocked, p);
			},
			[&](const Position& a, const Position& b) {
				return costFunc(a, b);
			},
			[&](const Position& a) {
				return heurFunc(a, to);
			},
			[&](const Position& a) {
				return goalTestFunc(a, to);
			},
			from);
}

std::set<Common::Position> AStar::graphFunc(const std::set<Common::Position>& blocked,
	const Common::Position& a) const
{
	std::set<Common::Position> ret;

	if(!mMapData) {
		std::cerr << "No map data.\n";
		return ret;
	}

	if(a.x > 0) {
		if(a.y > 0) {
			maybeInsert(ret, blocked, Position(a.x - 1, a.y - 1));
		}
		maybeInsert(ret, blocked, Position(a.x - 1, a.y));
		if(a.y < mMapData->getHeight() - 1) {
			maybeInsert(ret, blocked, Position(a.x - 1, a.y + 1));
		}
	}
	if(a.x < mMapData->getWidth() - 1) {
		if(a.y > 0) {
			maybeInsert(ret, blocked, Position(a.x + 1, a.y - 1));
		}
		maybeInsert(ret, blocked, Position(a.x + 1, a.y));
		if(a.y < mMapData->getHeight() - 1) {
			maybeInsert(ret, blocked, Position(a.x + 1, a.y + 1));
		}
	}
	if(a.y > 0) {
		maybeInsert(ret, blocked, Position(a.x, a.y - 1));
	}
	if(a.y < mMapData->getHeight() - 1) {
		maybeInsert(ret, blocked, Position(a.x, a.y + 1));
	}
	return ret;
}

void AStar::maybeInsert(std::set<Common::Position>& s, const std::set<Common::Position>& blocked,
		const Common::Position& p) const
{
	auto fr = mMapData->getPoint(p.x, p.y);
	if(!fr.wall && fr.vegetationlevel == Common::VegetationLevel::None) {
		if(blocked.find(p) == blocked.end()) {
			s.insert(p);
		}
	}
}

int AStar::costFunc(const Common::Position& a, const Common::Position& b) const
{
	return mMapData->movementCost(b);
}

int AStar::heurFunc(const Common::Position& from, const Common::Position& to) const
{
	int xdiff = abs(int(from.x) - int(to.x));
	int ydiff = abs(int(from.y) - int(to.y));
	return xdiff + ydiff;
}

bool AStar::goalTestFunc(const Common::Position& node, const Common::Position& goal) const
{
	return node == goal;
}


}

}

