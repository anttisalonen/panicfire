#include <stdexcept>
#include <iostream>

#include "common/Structures.h"
#include "game/Game.h"

using namespace PanicFire;

int main(void)
{
	try {
		Game g;
		g.run();
	}
	catch (std::exception& e) {
		std::cerr << "std::exception: " << e.what() << "\n";
	}
	catch(...) {
		std::cerr << "Unknown exception.\n";
	}
	return 0;
}

