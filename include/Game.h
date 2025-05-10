#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "Dinosaur.h"
#include "Obstacle.h"
#include "Bird.h"
#include "InstructionText.h"
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

struct Cloud
{
    Texture2D texture;
    Vector2 position;
    float speed;
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

    // 资源纹理
    std::vector<Texture2D> dinoRunFrames;
    std::vector<Texture2D> dinoSneakFrames;
    std::vector<Texture2D> smallCactusTextures;
    std::vector<Texture2D> bigCactusTextures;
    std::vector<Texture2D> roadSegmentTextures;
    std::vector<Texture2D> birdFrames;
    Texture2D dinoDeadTexture;
    Texture2D cloudTexture;

    // 音频资源
    Sound jumpSound;
    Sound dashSound;
    Sound deadSound;
    Sound bombSound;
    Music bgmMusic;

    std::deque<Road> activeRoadSegments;

    std::deque<Cloud> activeClouds; // 存储当前屏幕上的云彩
    float cloudSpawnTimerValue; // 云彩生成计时器
    float nextCloudSpawnTime;

    InstructionText jumpInstruction;
    bool instructionHasBeenTriggeredThisSession;
    float instructionActivationDelayTimer; // <--- 新增：用于延迟激活提示的计时器
    const float INSTRUCTION_ACTIVATION_DELAY = 2.0f; // <--- 新增：延迟2秒后显示提示
    bool instructionDelayPhaseActive; // <--- 新增：标记是否处于延迟激活阶段


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
    void InitRoadSegments();
    void UpdateRoadSegments(float deltaTime);
    void DrawRoads() const;
    void SpawnCloud();
    void UpdateClouds(float deltaTime);
    void DrawClouds() const;
};

#endif // GAME_H
