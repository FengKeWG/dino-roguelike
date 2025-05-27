#ifndef SWORD_H
#define SWORD_H

#include "raylib.h"
#include "Dinosaur.h" // 剑需要知道它的所有者（恐龙）的位置和朝向
#include "Bird.h"     // 用于碰撞检测
#include "ParticleSystem.h" // 用于击中时的粒子效果
#include <vector>
#include <cmath> // For std::fabs
#include "raymath.h" // For Lerp

class Sword
{
public:
    // 构造函数，传入剑的纹理、挥剑声音和恐龙主人
    Sword(Texture2D tex, Sound sound, Dinosaur* ownerDino);
    ~Sword();

    void Update(float deltaTime); // 更新剑的状态（例如攻击动画）
    void Draw() const; // 绘制剑
    void Attack(); // 触发攻击动作
    bool IsAttacking() const; // 返回剑当前是否正在攻击

    // 处理剑与鸟的碰撞
    // - birds: 游戏中所有鸟的列表 (会被修改，如果鸟被击中)
    // - gameScore: 游戏分数 (会被修改，如果击中鸟)
    // - effectParticles: 用于播放击中效果的粒子系统
    // - effectProps: 粒子效果的属性
    // - worldScrollSpeed: 当前世界滚动速度，用于粒子效果
    void CheckCollisionsWithBirds(std::vector<Bird>& birds, int& gameScore,
                                  ParticleSystem& effectParticles,
                                  const ParticleProperties& effectProps,
                                  float worldScrollSpeed, const Sound& birdScreamSound);

private:
    Texture2D texture; // 剑的纹理
    Sound swingSound; // 挥剑的声音
    Dinosaur* owner; // 指向持有剑的恐龙

    float cooldownTimer;
    float attackCooldown;

    bool isAttackingState; // 标记是否正在攻击
    float attackTimer; // 攻击动画计时器
    float attackDuration; // 攻击动画总时长

    float currentVisualRotation; // 剑当前的视觉旋转角度

    // 剑的属性 (可以在构造函数中配置，或设为常量)
    Vector2 pivotInTexture; // 纹理自身的旋转轴心点 (例如，对于170x170的图，可能是 {25.0f, 150.0f})
    float drawScale; // 绘制时的缩放比例 (例如, 0.5f)
    float textureInitialAngle; // 剑刃在原始图片中的角度 (例如，45.0f，指向右上)
    float swingStartAngleWorld; // 挥砍相对于主人的起始角度 (例如，-90.0f 表示从正上方)
    float swingEndAngleWorld; // 挥砍相对于主人的结束角度 (例如， 0.0f 表示挥向正前方)

    Rectangle GetSwordAABB() const; // 获取剑的轴对齐包围盒，用于简单碰撞检测
    Vector2 GetAttachmentPoint() const; // 获取剑在恐龙身上的附着点（旋转中心）
};

#endif // SWORD_H
