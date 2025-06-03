// include/Sword.h
#ifndef SWORD_H
#define SWORD_H

#include "raylib.h"
#include "Dinosaur.h"
#include "Bird.h"
#include "ParticleSystem.h"
#include <vector>
#include <cmath>
#include "raymath.h"

// 表示恐龙的剑武器
class Sword
{
public:
    // 构造函数
    Sword(const Texture2D& tex, const Sound& sound, Dinosaur* ownerDino);
    // 析构函数
    ~Sword();

    // 更新剑的状态和动画
    void Update(float deltaTime);
    // 绘制剑
    void Draw() const;
    // 执行攻击动作
    void Attack();
    // 检查剑是否正在攻击状态
    bool IsAttacking() const;
    // 检测剑与鸟群的碰撞
    void CheckCollisionsWithBirds(std::vector<Bird>& birds, int& gameScore,
                                  ParticleSystem& effectParticles,
                                  const ParticleProperties& effectProps,
                                  float worldScrollSpeed, const Sound& birdScreamSound) const;

    // 检查剑是否在冷却中
    bool IsOnCooldown() const;
    // 获取冷却进度 (0.0 到 1.0)
    float GetCooldownProgress() const;

private:
    Texture2D texture; // 剑的纹理
    Sound swingSound; // 挥剑音效
    Dinosaur* owner; // 剑的拥有者 (恐龙)

    float cooldownTimer; // 冷却计时器
    float attackCooldown; // 攻击冷却时间

    bool isAttackingState; // 是否处于攻击状态
    float attackTimer; // 攻击动画计时器
    float attackDuration; // 攻击动画总时长
    float currentVisualRotation; // 当前剑的视觉旋转角度 (用于绘制)

    Vector2 pivotInTexture; // 纹理的旋转轴心点 (相对于纹理左上角)
    float drawScale; // 绘制时的缩放比例
    float textureInitialAngle; // 纹理本身的初始角度 (如果美术资源不是水平的)
    float swingStartAngleWorld; // 挥动开始时的世界角度
    float swingEndAngleWorld; // 挥动结束时的世界角度

    // 获取剑的碰撞箱
    Rectangle GetSwordAABB() const;
    // 获取剑在恐龙身上的附着点/旋转中心点
    Vector2 GetAttachmentPoint() const;
};

#endif // SWORD_H
