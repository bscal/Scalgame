#include "src/core/Game.h"
#include "Server.h"

int main()
{
    GameSpace::SetupServer();

    Game::GetInstance().Start();

    return 0;
}
