#ifndef PANICFIRE_UI_DRIVER_H
#define PANICFIRE_UI_DRIVER_H

#include "common/DriverFramework.h"

#include "panicfire/common/Structures.h"

namespace PanicFire {

namespace UI {

class Driver : public ::Common::Driver {

	public:
		Driver(Common::WorldInterface& w);
		void run();

	protected:
		bool init() override;

	private:
		Common::WorldInterface& mWorld;
};

}

}

#endif

