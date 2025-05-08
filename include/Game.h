#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "Dinosaur.h"
#include "Obstacle.h"
#include "Bird.h"
#include <vector>
#include <deque>

enum class GameState
{
    PLAYING,
    GAME_OVER
};

class Game
{
public:
    Game(int screenWidth, int screenHeight, const char* title);
    ~Game();
    void Run();

private:
    int screenWidth;
    int screenHeight;
    const int virtualScreenWidth = 960;
    const int virtualScreenHeight = 540;
    RenderTexture2D targetRenderTexture;
    Rectangle sourceRec;
    Rectangle destRec;
    Vector2 origin;

    bool isFakeFullscreen;
    int windowedPosX, windowedPosY;
    int windowedWidth, windowedHeight;

    Dinosaur* dino;
    std::vector<Obstacle> obstacles;
    std::vector<Bird> birds;

    GameState currentState;
    float groundY;
    float timePlayed;
    int score;

    float worldBaseScrollSpeed;
    float currentWorldScrollSpeed;
    float worldSpeedIncreaseRate;

    float obstacleSpawnTimer;
    float minObstacleSpawnInterval;
    float maxObstacleSpawnInterval;
    float currentObstacleSpawnInterval;

    // 资源纹理 (无 idle, 无 jump)
    std::vector<Texture2D> dinoRunFrames;
    std::vector<Texture2D> dinoSneakFrames;
    std::vector<Texture2D> smallCactusTextures;
    std::vector<Texture2D> bigCactusTextures;
    std::vector<Texture2D> roadSegmentTextures;
    std::vector<Texture2D> birdFrames;

    // 音频资源
    Sound jumpSound;
    Music bgmMusic;

    struct RoadSegment
    {
        Texture2D texture;
        float xPosition;
    };

    std::deque<RoadSegment> activeRoadSegments;

    void InitGame();
    void UpdateGame(float deltaTime);
    void DrawGame();
    void HandleInput();
    void SpawnObstacleOrBird();
    void CheckCollisions();
    void ResetGame();
    void LoadResources();
    void UnloadResources();
    void HandleWindowResize();
    void UpdateRenderTextureScaling();
    void InitRoadSegments();
    void UpdateRoadSegments(float deltaTime);
    void DrawRoadSegments() const;
};

#endif // GAME_H
