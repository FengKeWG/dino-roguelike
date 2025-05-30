// src/ParticleSystem.cpp
#include "../include/ParticleSystem.h"
#include <cstdlib>
#include <cmath>

ParticleSystem::ParticleSystem(const int maxParticlesCount)
    : poolIndex(0)
{
    particlesPool.resize(maxParticlesCount);
    systemGravity = {0, 980.0f};
}

ParticleSystem::~ParticleSystem()
{
    particlesPool.clear();
}

void ParticleSystem::SetGravity(const Vector2 newGravity)
{
    systemGravity = newGravity;
}

void ParticleSystem::Reset()
{
    for (auto& p : particlesPool)
    {
        p.isActive = false;
    }
    poolIndex = 0;
}

// 获取当前激活的粒子数量
int ParticleSystem::GetActiveParticlesCount() const
{
    int count = 0;
    for (const auto& p : particlesPool)
    {
        if (p.isActive) count++;
    }
    return count;
}

// 从指定位置发射指定数量的粒子
void ParticleSystem::Emit(const Vector2 emitterPosition, const int count, const ParticleProperties& props,
                          const float worldScrollSpeedX)
{
    for (int i = 0; i < count; ++i) // 发射指定数量的粒子
    {
        auto&
        [position, velocity, color, size, lifeTime, lifeRemaining, rotation, angularVelocity, gravityEffect,
            isActive, isOnGround, groundYLevel, groundScrollSpeedX] = particlesPool[poolIndex]; // 从对象池中获取一个粒子 (循环使用)
        poolIndex = (poolIndex + 1) % particlesPool.size();

        isActive = true;
        position = emitterPosition;
        // 生命周期
        lifeTime = randF(props.lifeTimeMin, props.lifeTimeMax);
        lifeRemaining = lifeTime; // 剩余生命等于总生命
        // 根据属性范围随机设置发射角度和速度
        const float angleRad = randF(props.emissionAngleMin, props.emissionAngleMax) * DEG2RAD; // 角度转弧度
        const float speed = randF(props.initialSpeedMin, props.initialSpeedMax);
        velocity.x = cosf(angleRad) * speed; // 计算X方向速度
        velocity.y = sinf(angleRad) * speed; // 计算Y方向速度
        size = randF(props.startSizeMin, props.startSizeMax); // 随机大小
        color = props.startColor; // 设置颜色
        rotation = 0.0f; // 初始旋转为0
        angularVelocity = randF(props.angularVelocityMin, props.angularVelocityMax); // 随机角速度
        gravityEffect = randF(props.gravityScaleMin, props.gravityScaleMax); // 随机重力影响因子
        isOnGround = false;
        groundYLevel = props.targetGroundY; // 设置目标地面Y坐标
        groundScrollSpeedX = -worldScrollSpeedX; // 粒子在地面上时，随世界反向滚动
    }
}

// 更新所有激活粒子的状态
void ParticleSystem::Update(const float deltaTime)
{
    for (auto& p : particlesPool)
    {
        if (!p.isActive) continue;

        p.lifeRemaining -= deltaTime; // 减少剩余生命
        if (p.lifeRemaining <= 0.0f)
        {
            p.isActive = false;
            continue;
        }

        if (p.isOnGround)
        {
            p.position.x += p.groundScrollSpeedX * deltaTime; // 随地面滚动
            p.velocity.y = 0;
            p.angularVelocity *= 0.95f; // 角速度逐渐减小
            if (std::abs(p.angularVelocity) < 0.1f) p.angularVelocity = 0;
        }
        else
        {
            p.velocity.x += systemGravity.x * p.gravityEffect * deltaTime;
            p.velocity.y += systemGravity.y * p.gravityEffect * deltaTime;
            p.position.x += p.velocity.x * deltaTime;
            p.position.y += p.velocity.y * deltaTime;
            p.rotation += p.angularVelocity * deltaTime;

            if (p.groundYLevel > 0 && p.position.y + p.size / 2.0f >= p.groundYLevel && p.velocity.y > 0)
            {
                p.isOnGround = true;
                p.position.y = p.groundYLevel - p.size / 2.0f;
            }
        }
    }
}

void ParticleSystem::Draw() const
{
    for (const auto& p : particlesPool)
    {
        if (!p.isActive) continue;
        DrawRectanglePro(
            {p.position.x, p.position.y, p.size, p.size},
            {p.size / 2, p.size / 2},
            p.rotation,
            p.color
        );
    }
}
