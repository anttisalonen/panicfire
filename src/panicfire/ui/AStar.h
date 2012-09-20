#ifndef PANICFIRE_UI_ASTAR_H
#define PANICFIRE_UI_ASTAR_H

#include <list>
#include <set>

#include "panicfire/common/Structures.h"

namespace PanicFire {

namespace UI {

class AStar {
	public:
		AStar();
		void setMapData(const Common::MapData* m);
		std::list<Common::Position> solve(const std::set<Common::Position>& blocked,
				const Common::Position& from,
				const Common::Position& to) const;

	private:
		const Common::MapData* mMapData;

		std::set<Common::Position> graphFunc(const std::set<Common::Position>& blocked,
				const Common::Position& a) const;
		int costFunc(const Common::Position& a, const Common::Position& b) const;
		int heurFunc(const Common::Position& from, const Common::Position& to) const;
		bool goalTestFunc(const Common::Position& node, const Common::Position& goal) const;

		void maybeInsert(std::set<Common::Position>& s, const std::set<Common::Position>& blocked,
				const Common::Position& p) const;
};

}

}


#endif

