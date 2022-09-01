#pragma once

#include "Game.h"

int main(int argc, char** argv)
{
	GameApplication app = {};
	app.Start();
	app.Run();
	app.Shutdown();
	return 0;
}