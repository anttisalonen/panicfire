#include <stdexcept>
#include <iostream>

#include "game/World.h"
#include "ui/Driver.h"

using namespace PanicFire;

int main(void)
{
	try {
		Game::World w;
		UI::Driver d(w);
		d.run();
	}
	catch (std::exception& e) {
		std::cerr << "std::exception: " << e.what() << "\n";
	}
	catch(...) {
		std::cerr << "Unknown exception.\n";
	}
	return 0;
}

