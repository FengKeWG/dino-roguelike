#include "../include/Game.h"

int main()
{
    const int initialScreenWidth = 960;
    const int initialScreenHeight = 540;

    Game game(initialScreenWidth, initialScreenHeight, "Dino Plus Ultra");
    game.Run();

    return 0;
}
