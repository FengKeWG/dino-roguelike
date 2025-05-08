#include "../include/Game.h"

int main() {
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 450;

    Game game(screenWidth, screenHeight, "Google Dino Clone");
    game.Run();

    return 0;
}
