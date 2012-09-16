#include <iostream>

#include "panicfire/game/Game.h"

namespace PanicFire { 

using namespace PanicFire::Common;

Game::Game()
{
}

Common::QueryResult Game::query(const Common::Query& q)
{
	return PanicFire::Common::DeniedQueryResult();
}

bool Game::input(const Common::Input& i)
{
	return false;
}

Common::Event Game::pollEvents()
{
	return EmptyEvent();
}

void Game::run()
{
	std::cout << "Hello world!\n";
}

}

