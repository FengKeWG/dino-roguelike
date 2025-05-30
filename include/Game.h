// include/Game.h
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

// 游戏状态
enum class GameState
{
    PLAYING, // 游戏中
    GAME_OVER, // 游戏结束
    PAUSED // 暂停
};

// 路面
struct Road
{
    Texture2D texture; // 路面纹理
    float xPosition; // 路面X轴位置
};

class Game
{
public:
    Game(int width, int height, const char* title);
    ~Game();
    void Run();

private:
    int screenWidth; // 屏幕宽度
    int screenHeight; // 屏幕高度
    const int virtualScreenWidth; // 虚拟屏幕宽度 (用于缩放)
    const int virtualScreenHeight; // 虚拟屏幕高度 (用于缩放)
    RenderTexture2D targetRenderTexture; // 渲染目标纹理，用于实现缩放
    Rectangle sourceRec; // 渲染纹理的源矩形
    Rectangle destRec; // 渲染纹理的目标矩形
    Vector2 origin; // 渲染纹理的原点

    bool isFullscreen; // 是否全屏
    int windowedPosX, windowedPosY; // 窗口模式下的位置
    int windowedWidth, windowedHeight; // 窗口模式下的尺寸

    Rectangle pause_restartButtonRect; // 暂停界面-重新开始按钮区域
    Rectangle pause_exitButtonRect; // 暂停界面-退出按钮区域

    Dinosaur* dino; // 玩家恐龙对象指针
    Sword* playerSword; // 玩家的剑对象指针
    std::vector<Obstacle> obstacles; // 障碍物列表
    std::vector<Bird> birds; // 鸟类敌人列表

    GameState currentState; // 当前游戏状态
    float groundY; // 地面Y坐标
    float timePlayed; // 游戏已进行时间
    int score; // 当前得分

    float worldBaseScrollSpeed; // 世界基础滚动速度
    float currentWorldScrollSpeed; // 当前世界滚动速度
    float worldSpeedIncreaseRate; // 世界滚动速度增长率

    float obstacleSpawnTimer; // 障碍物生成计时器
    float minObstacleSpawnInterval; // 最小障碍物生成间隔
    float maxObstacleSpawnInterval; // 最大障碍物生成间隔
    float currentObstacleSpawnInterval; // 当前计算出的障碍物生成间隔

    // 资源纹理
    std::vector<Texture2D> dinoRunFrames; // 恐龙奔跑动画帧
    std::vector<Texture2D> dinoSneakFrames; // 恐龙潜行动画帧
    std::vector<Texture2D> smallCactusTextures; // 小仙人掌纹理
    std::vector<Texture2D> bigCactusTextures; // 大仙人掌纹理
    std::vector<Texture2D> roadSegmentTextures; // 路面片段纹理
    std::vector<Texture2D> birdFrames; // 鸟飞行帧
    Texture2D dinoDeadTexture; // 恐龙死亡纹理
    Texture2D cloudTexture; // 云彩纹理
    Texture2D swordTexture; // 剑的纹理

    // 音频资源
    Sound jumpSound; // 跳跃音效
    Sound dashSound; // 冲刺音效
    Sound deadSound; // 死亡音效
    Sound bombSound; // 爆炸音效
    Sound swordSound; // 挥剑音效
    Sound screamSound; // 鸟叫声音效
    Music bgmMusic; // 背景音乐

    ParticleSystem birdDeathParticles; // 鸟死亡粒子系统
    ParticleProperties birdDeathParticleProps; // 鸟死亡粒子属性

    std::deque<Road> activeRoadSegments; // 当前激活的路面片段
    std::deque<Cloud> activeClouds; // 存储当前屏幕上的云彩
    float cloudSpawnTimerValue; // 云彩生成计时器
    float nextCloudSpawnTime; // 下一次生成云彩的时间

    InstructionManager instructionManager; // 教学提示管理器

    // 初始化游戏
    void InitGame();
    // 更新游戏逻辑
    void UpdateGame(float deltaTime);
    // 绘制游戏画面
    void DrawGame() const;
    // 处理用户输入
    void HandleInput();
    // 生成障碍物或鸟
    void SpawnObstacleOrBird();
    // 检测碰撞
    void CheckCollisions();
    // 重置游戏状态
    void ResetGame();
    // 加载所有游戏资源
    void LoadResources();
    // 卸载所有游戏资源
    void UnloadResources();
    // 处理窗口大小改变事件
    void HandleWindowResize();
    // 更新渲染纹理的缩放参数
    void UpdateRenderTextureScaling();
    // 初始化路面
    void InitRoads();
    // 更新路面片段
    void UpdateRoadSegments(float deltaTime);
    // 生成云彩
    void SpawnCloud();
    // 更新云彩
    void UpdateClouds(float deltaTime);
};

#endif // GAME_H
