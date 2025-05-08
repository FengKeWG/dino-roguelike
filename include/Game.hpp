#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "Dinosaur.hpp"
#include "Obstacle.hpp"
#include <vector>
#include <string>
#include <deque> // For road segments

enum class GameState {
    PLAYING,
    GAME_OVER,
    // MENU, // Future
    // PAUSED // Future
};

class Game {
public:
    Game(int screenWidth, int screenHeight, const char *title);

    ~Game();

    void Run();

private:
    int screenWidth;
    int screenHeight;

    Dinosaur *dino;
    std::vector<Obstacle> obstacles;

    GameState currentState;
    float groundY;
    float gameSpeed;
    int score;
    float obstacleSpawnTimer;
    float minObstacleSpawnInterval;
    float maxObstacleSpawnInterval;
    float currentObstacleSpawnInterval; // 当前实际的生成间隔

    // 资源
    std::vector<Texture2D> dinoRunFrames;
    Texture2D dinoJumpFrame;
    std::vector<Texture2D> smallCactusTextures; // small_cactus_1, small_cactus_2, small_cactus_3
    std::vector<Texture2D> bigCactusTextures; // big_cactus_1, big_cactus_2
    std::vector<Texture2D> roadSegmentTextures; // road_1, road_2, road_3, road_4

    // 地面滚动相关
    struct RoadSegment {
        Texture2D texture;
        float xPosition;
    };

    std::deque<RoadSegment> activeRoadSegments;
    float totalRoadWidthCovered; // 用于判断何时添加新的路段

    void InitGame();

    void UpdateGame(float deltaTime);

    void DrawGame();

    void HandleInput();

    void SpawnObstacle();

    void CheckCollisions();

    void ResetGame();

    void LoadResources();

    void UnloadResources();

    void HandleWindowResize();

    void InitRoadSegments(); // 初始化地面段
    void UpdateRoadSegments(float deltaTime); // 更新地面段滚动
    void DrawRoadSegments(); // 绘制地面段

    // TODO: 扩展元素占位
};

#endif // GAME_H
