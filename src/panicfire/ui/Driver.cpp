#include <iostream>

#include "panicfire/ui/Driver.h"

using namespace PanicFire;

namespace PanicFire {

namespace UI {

Driver::Driver(Common::WorldInterface& w)
	: ::Common::Driver(800, 600, "Panic Fire"),
	mWorld(w)
{
}

bool Driver::init()
{
	return true;
}

void Driver::run()
{
	std::cout << "Hello world!\n";
}


}

}

