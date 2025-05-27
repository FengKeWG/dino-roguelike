// ParticleSystem.cpp
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

int ParticleSystem::GetActiveParticlesCount() const
{
    int count = 0;
    for (const auto& p : particlesPool)
    {
        if (p.isActive) count++;
    }
    return count;
}

// 修改 Emit 函数以接收 worldScrollSpeedX
void ParticleSystem::Emit(Vector2 emitterPosition, int count, const ParticleProperties& props, float worldScrollSpeedX)
{
    for (int i = 0; i < count; ++i)
    {
        Particle& p = particlesPool[poolIndex];
        poolIndex = (poolIndex + 1) % particlesPool.size();

        p.isActive = true;
        p.position = emitterPosition;
        p.lifeTime = randF(props.lifeTimeMin, props.lifeTimeMax);
        p.lifeRemaining = p.lifeTime;

        float angleRad = randF(props.emissionAngleMin, props.emissionAngleMax) * DEG2RAD;
        float speed = randF(props.initialSpeedMin, props.initialSpeedMax);
        p.velocity.x = cosf(angleRad) * speed;
        p.velocity.y = sinf(angleRad) * speed;

        p.size = randF(props.startSizeMin, props.startSizeMax);
        p.color = props.startColor; // Alpha直接使用startColor的alpha

        p.rotation = 0.0f;
        p.angularVelocity = randF(props.angularVelocityMin, props.angularVelocityMax);
        p.gravityEffect = randF(props.gravityScaleMin, props.gravityScaleMax);

        // 设置地面交互属性
        p.isOnGround = false;
        p.groundYLevel = props.targetGroundY;
        p.groundScrollSpeedX = -worldScrollSpeedX; // 粒子在地面上时，X方向与世界滚动反向
    }
}

void ParticleSystem::Update(float deltaTime)
{
    for (auto& p : particlesPool)
    {
        if (!p.isActive) continue;

        p.lifeRemaining -= deltaTime;
        if (p.lifeRemaining <= 0.0f)
        {
            p.isActive = false;
            continue;
        }

        if (p.isOnGround)
        {
            // 如果在地面上，只受地面滚动影响 (X轴)
            p.position.x += p.groundScrollSpeedX * deltaTime;
            // Y轴速度和角速度可以减缓或停止
            p.velocity.y = 0;
            p.angularVelocity *= 0.95f; // 模拟摩擦力减慢旋转
            if (std::abs(p.angularVelocity) < 0.1f) p.angularVelocity = 0;
        }
        else
        {
            // 不在地面上，正常物理更新
            p.velocity.x += systemGravity.x * p.gravityEffect * deltaTime;
            p.velocity.y += systemGravity.y * p.gravityEffect * deltaTime;

            p.position.x += p.velocity.x * deltaTime;
            p.position.y += p.velocity.y * deltaTime;

            p.rotation += p.angularVelocity * deltaTime;

            // 检测是否到达地面
            if (p.groundYLevel > 0 && p.position.y + p.size / 2.0f >= p.groundYLevel && p.velocity.y > 0)
            {
                p.isOnGround = true;
                p.position.y = p.groundYLevel - p.size / 2.0f; // 精确停在地面
                // 可以在这里改变X速度，让它等于地面滚动速度
                // p.velocity.x = p.groundScrollSpeedX; // 立即改变X速度，或者让上面的逻辑处理
            }
        }
        // Alpha值保持不变，直到生命结束
        // p.color.a = p.initialColor.a; (实际上在Emit时已经设置了)
    }
}

void ParticleSystem::Draw() const // Draw函数不变
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
