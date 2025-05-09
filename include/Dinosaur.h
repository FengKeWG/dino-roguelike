#ifndef DINOSAUR_H
#define DINOSAUR_H

#include "raylib.h"
#include <vector>
#include <string> // 良好包含习惯
#include "raymath.h" // 用于 Vector2 操作

// 冲刺粒子效果结构体
struct DashParticle
{
    Vector2 position;
    Vector2 velocity;
    Color color;
    float lifeRemaining;
    float initialLife;
    float size;
    float rotation;
    float angularVelocity;
    float gravityEffect; // 每个粒子受到的重力影响程度
    bool isLanded; // <--- 新增：是否已固定在地面
    float landedScrollSpeedX; // <--- 新增：落地后随世界滚动的X速度
};

class Dinosaur
{
public:
    // 公共成员
    Vector2 position;
    Vector2 velocity; // y用于跳跃/重力, x用于普通移动
    float groundY;
    float runHeight; // 跑动时的高度
    float sneakHeight; // 潜行时的高度

    // 构造函数 & 析构函数
    Dinosaur(float startX, float initialGroundY,
             const std::vector<Texture2D>& runTex,
             const std::vector<Texture2D>& sneakTex,
             const Sound& jumpSound,
             const Sound& rushSound);
    ~Dinosaur();

    //核心方法
    void Update(float deltaTime, float worldScrollSpeed);
    void Draw() const;

    // 行为
    void RequestJump();
    void StartSneaking();
    void StopSneaking();
    void Move(float direction, float deltaTime); // 水平移动输入
    void RequestDash(); // 新增：冲刺请求

    // Getters
    Rectangle GetCollisionRect() const;
    float GetWidth() const;
    float GetHeight() const;
    bool IsOnGround() const;
    bool IsSneaking() const { return isSneaking; }
    bool IsJumping() const { return isJumping; }
    bool IsFacingRight() const { return facingRight; } // 获取朝向
    void UpdateCollisionRect();

private:
    // 内部状态
    Sound jumpSoundHandle;
    Sound dashSoundHandle;
    bool isJumping;
    bool isSneaking;
    bool facingRight; // true代表朝右

    // 动画
    std::vector<Texture2D> runFrames;
    std::vector<Texture2D> sneakFrames;
    int currentAnimFrameIndex;
    float frameTimeCounter;
    float animationSpeed;

    // 碰撞
    Rectangle collisionRect; // 原始包围盒

    // 跳跃机制
    float gravity;
    float jumpSpeed;
    float coyoteTimeDuration = 0.1f;
    float coyoteTimeCounter;
    float jumpBufferDuration = 0.1f;
    float jumpBufferCounter;
    bool jumpQueued;
    float sneakGravityMultiplier;

    // 移动
    float moveSpeed;

    // --- 新增冲刺属性 ---
    bool isDashing;
    float dashSpeedMagnitude; // 冲刺速度大小
    float dashDuration; // 冲刺持续时间
    float dashTimer; // 当前冲刺已进行时间
    float dashCooldown; // 冲刺冷却时间
    float dashCooldownTimer; // 当前冷却剩余时间
    Vector2 dashDirection; // 冲刺方向 (主要用 x)

    // 粒子系统
    std::vector<DashParticle> particles;
    float particleBaseGravity; // <--- 粒子基础重力

    // 私有辅助方法
    void ExecuteJump();
    const std::vector<Texture2D>* GetCurrentAnimationFramesPointer() const;
    Texture2D GetCurrentTextureToDraw() const;

    // 新增冲刺和粒子相关的私有方法
    void EmitDashParticleTrail(float currentWorldScrollSpeed);
    void UpdateParticles(float deltaTime, float worldScrollSpeed);
    void DrawParticles() const;
};

#endif // DINOSAUR_H
