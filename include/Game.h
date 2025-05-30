#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "Dinosaur.h"
#include "Obstacle.h"
#include "Cloud.h"
#include "Sword.h"
#include "Bird.h"
#include "InstructionManager.h"
#include <vector>
#include <deque>

enum class GameState
{
    PLAYING,
    GAME_OVER,
    PAUSED
};

struct Road
{
    Texture2D texture;
    float xPosition;
};

class Game
{
public:
    Game(int width, int height, const char* title);
    ~Game();
    void Run();

private:
    int screenWidth;
    int screenHeight;
    const int virtualScreenWidth = 960.0f;
    const int virtualScreenHeight = 540.0f;
    RenderTexture2D targetRenderTexture;
    Rectangle sourceRec;
    Rectangle destRec;
    Vector2 origin;

    bool isFullscreen;
    int windowedPosX, windowedPosY;
    int windowedWidth, windowedHeight;

    Rectangle pause_restartButtonRect;
    Rectangle pause_exitButtonRect;

    Dinosaur* dino;
    Sword* playerSword;
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

    // 资源纹理
    std::vector<Texture2D> dinoRunFrames;
    std::vector<Texture2D> dinoSneakFrames;
    std::vector<Texture2D> smallCactusTextures;
    std::vector<Texture2D> bigCactusTextures;
    std::vector<Texture2D> roadSegmentTextures;
    std::vector<Texture2D> birdFrames;
    Texture2D dinoDeadTexture;
    Texture2D cloudTexture;
    Texture2D swordTexture;

    // 音频资源
    Sound jumpSound;
    Sound dashSound;
    Sound deadSound;
    Sound bombSound;
    Sound swordSound;
    Sound screamSound;
    Music bgmMusic;

    ParticleSystem birdDeathParticles;
    ParticleProperties birdDeathParticleProps;

    std::deque<Road> activeRoadSegments;
    std::deque<Cloud> activeClouds; // 存储当前屏幕上的云彩
    float cloudSpawnTimerValue; // 云彩生成计时器
    float nextCloudSpawnTime;

    InstructionManager instructionManager;

    void InitGame();
    void UpdateGame(float deltaTime);
    void DrawGame() const;
    void HandleInput();
    void SpawnObstacleOrBird();
    void CheckCollisions();
    void ResetGame();
    void LoadResources();
    void UnloadResources();
    void HandleWindowResize();
    void UpdateRenderTextureScaling();
    void InitRoads();
    void UpdateRoadSegments(float deltaTime);

    void SpawnCloud();
    void UpdateClouds(float deltaTime);
};

#endif // GAME_H
