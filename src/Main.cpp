#include "../include/Game.h"

int main()
{
    constexpr int initialScreenWidth = 960;
    constexpr int initialScreenHeight = 540;
    Game game(initialScreenWidth, initialScreenHeight, "Dino Plus Ultra");
    game.Run();
    return 0;
}
