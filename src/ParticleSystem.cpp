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


void ParticleSystem::Emit(const Vector2 emitterPosition, const int count, const ParticleProperties& props,
                          const float worldScrollSpeedX)
{
    for (int i = 0; i < count; ++i)
    {
        Particle& p = particlesPool[poolIndex];
        poolIndex = (poolIndex + 1) % particlesPool.size();
        p.isActive = true;
        p.position = emitterPosition;
        p.lifeTime = randF(props.lifeTimeMin, props.lifeTimeMax);
        p.lifeRemaining = p.lifeTime;
        const float angleRad = randF(props.emissionAngleMin, props.emissionAngleMax) * DEG2RAD;
        const float speed = randF(props.initialSpeedMin, props.initialSpeedMax);
        p.velocity.x = cosf(angleRad) * speed;
        p.velocity.y = sinf(angleRad) * speed;
        p.size = randF(props.startSizeMin, props.startSizeMax);
        p.color = props.startColor;
        p.rotation = 0.0f;
        p.angularVelocity = randF(props.angularVelocityMin, props.angularVelocityMax);
        p.gravityEffect = randF(props.gravityScaleMin, props.gravityScaleMax);
        p.isOnGround = false;
        p.groundYLevel = props.targetGroundY;
        p.groundScrollSpeedX = -worldScrollSpeedX;
    }
}

void ParticleSystem::Update(const float deltaTime)
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
            p.position.x += p.groundScrollSpeedX * deltaTime;
            p.velocity.y = 0;
            p.angularVelocity *= 0.95f;
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
