// ParticleSystem.h
#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "raylib.h"
#include "Utils.h"
#include <vector>
#include <string>

// 单个粒子的定义
struct Particle
{
    Vector2 position;
    Vector2 velocity;
    Color color; // 颜色（Alpha将保持不变直到生命结束）
    float size;
    float lifeTime; // 总生命周期
    float lifeRemaining; // 剩余生命周期
    float rotation;
    float angularVelocity;
    float gravityEffect; // 粒子受重力影响的系数
    bool isActive;

    // --- 新增用于地面交互的属性 ---
    bool isOnGround; // 粒子是否已在地面上
    float groundYLevel; // 粒子应该停在的Y坐标 (由发射时设置)
    float groundScrollSpeedX; // 粒子在地面上时，X方向的滚动速度（通常是负的世界滚动速度）


    Particle() : position({0, 0}), velocity({0, 0}), color(WHITE), size(1.0f),
                 lifeTime(1.0f), lifeRemaining(0.0f), rotation(0.0f),
                 angularVelocity(0.0f), gravityEffect(1.0f), isActive(false),
                 isOnGround(false), groundYLevel(0.0f), groundScrollSpeedX(0.0f)
    {
    }
};

// 配置发射时粒子的属性
struct ParticleProperties
{
    // 生命周期
    float lifeTimeMin = 0.5f;
    float lifeTimeMax = 1.5f;

    // 初始速度
    float initialSpeedMin = 50.0f;
    float initialSpeedMax = 150.0f;

    // 发射角度 (0-360度, 0度为右方)
    float emissionAngleMin = 0.0f;
    float emissionAngleMax = 360.0f;

    // 大小
    float startSizeMin = 2.0f;
    float startSizeMax = 5.0f;

    // 颜色 (Alpha值会被保留)
    Color startColor = {255, 255, 255, 255}; // RGBA

    // 旋转
    float angularVelocityMin = -90.0f; // 度/秒
    float angularVelocityMax = 90.0f;

    // 重力影响
    float gravityScaleMin = 0.8f;
    float gravityScaleMax = 1.2f;

    // --- 地面交互属性 ---
    float targetGroundY = -1.0f; // 粒子应该停在的Y坐标，如果 < 0 则不考虑地面
    // groundScrollSpeedX 将在 ParticleSystem::Emit 中根据外部参数设置

    ParticleProperties() = default;
};


class ParticleSystem
{
public:
    ParticleSystem(int maxParticlesCount);
    ~ParticleSystem();

    void Update(float deltaTime);
    void Draw() const;

    // 发射粒子
    // emitterPosition: 发射中心点
    // count: 发射数量
    // props: 粒子属性配置
    // worldScrollSpeedX: 当前世界的X轴滚动速度 (用于粒子落地后跟随)
    void Emit(Vector2 emitterPosition, int count, const ParticleProperties& props, float worldScrollSpeedX = 0.0f);

    void SetGravity(Vector2 newGravity);
    void Reset();
    int GetActiveParticlesCount() const;

private:
    std::vector<Particle> particlesPool;
    int poolIndex;
    Vector2 systemGravity;
};

#endif // PARTICLE_SYSTEM_H
