// include/Dinosaur.h
#ifndef DINOSAUR_H
#define DINOSAUR_H

#include "raylib.h"
#include <vector>
#include <iostream>
#include <cmath>
#include "ParticleSystem.h"

// 表示玩家控制的恐龙角色
class Dinosaur
{
public:
    Vector2 position; // 恐龙的位置
    Vector2 velocity; // 恐龙的速度
    float groundY; // 地面Y坐标
    float runHeight; // 奔跑时的高度
    float sneakHeight; // 潜行时的高度

    // 构造函数
    Dinosaur(float startX, float groundY,
             const std::vector<Texture2D>& runTex,
             const std::vector<Texture2D>& sneakTex,
             const Texture2D& deadTex,
             const Sound& jumpSound,
             const Sound& dashSound);
    // 析构函数
    ~Dinosaur();

    // 更新恐龙状态
    void Update(float deltaTime, float worldScrollSpeed);
    // 绘制恐龙
    void Draw() const;

    // 请求跳跃
    void RequestJump();
    // 开始潜行
    void StartSneaking();
    // 停止潜行
    void StopSneaking();
    // 左右移动
    void Move(float direction, float deltaTime);
    // 请求冲刺
    void RequestDash();
    // 标记为死亡
    void MarkAsDead();

    // 获取碰撞矩形
    Rectangle GetCollisionRect() const;
    // 获取恐龙宽度
    float GetWidth() const;
    // 获取恐龙高度
    float GetHeight() const;
    // 是否在地面上
    bool IsOnGround() const;
    // 是否在潜行
    bool IsSneaking() const { return isSneaking; }
    // 是否在跳跃
    bool IsJumping() const { return isJumping; }
    // 是否朝向右边
    bool IsFacingRight() const { return facingRight; }

    // 更新碰撞矩形
    void UpdateCollisionRect();

private:
    Sound jumpSoundHandle; // 跳跃音效句柄
    Sound dashSoundHandle; // 冲刺音效句柄
    bool isJumping; // 跳跃状态标志
    bool isSneaking; // 潜行状态标志
    bool facingRight; // 朝向标志 (true为右)

    std::vector<Texture2D> runFrames; // 奔跑动画帧
    std::vector<Texture2D> sneakFrames; // 潜行动画帧
    Texture2D deadTexture; // 死亡状态纹理
    bool isDead; // 死亡状态标志
    int currentAnimFrameIndex; // 当前动画帧索引
    float frameTimeCounter; // 动画帧计时器
    float animationSpeed; // 动画播放速度

    Rectangle collisionRect; // 碰撞矩形

    float gravity; // 重力加速度
    float jumpSpeed; // 跳跃初速度
    float jumpBufferDuration; // 跳跃缓冲持续时长
    float jumpBufferCounter; // 跳跃缓冲计时器
    bool jumpQueued; // 是否已缓存跳跃请求
    float sneakGravityMultiplier; // 潜行时重力倍增器
    float moveSpeed; // 移动速度

    bool isDashing; // 是否正在冲刺
    float dashSpeedMagnitude; // 冲刺速度大小
    float dashDuration; // 冲刺持续时间
    float dashTimer; // 冲刺计时器
    float dashCooldown; // 冲刺冷却时间
    float dashCooldownTimer; // 冲刺冷却计时器
    Vector2 dashDirection; // 冲刺方向

    ParticleSystem dashTrailParticles; // 冲刺拖尾粒子效果
    ParticleProperties dashParticleProps; // 冲刺拖尾粒子属性

    // 执行跳跃动作
    void ExecuteJump();
    // 获取当前动画帧组的指针
    const std::vector<Texture2D>* GetCurrentAnimationFramesPointer() const;
    // 获取当前要绘制的纹理
    Texture2D GetCurrentTextureToDraw() const;
};

#endif // DINOSAUR_H
