#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "raylib.h"
#include "Utils.h"
#include <vector>
#include <string>


struct Particle
{
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float lifeTime;
    float lifeRemaining;
    float rotation;
    float angularVelocity;
    float gravityEffect;
    bool isActive;


    bool isOnGround;
    float groundYLevel;
    float groundScrollSpeedX;

    Particle() : position({0, 0}), velocity({0, 0}), color(WHITE), size(1.0f),
                 lifeTime(1.0f), lifeRemaining(0.0f), rotation(0.0f),
                 angularVelocity(0.0f), gravityEffect(1.0f), isActive(false),
                 isOnGround(false), groundYLevel(0.0f), groundScrollSpeedX(0.0f)
    {
    }
};


struct ParticleProperties
{
    float lifeTimeMin = 0.5f;
    float lifeTimeMax = 1.5f;
    float initialSpeedMin = 50.0f;
    float initialSpeedMax = 150.0f;
    float emissionAngleMin = 0.0f;
    float emissionAngleMax = 360.0f;
    float startSizeMin = 2.0f;
    float startSizeMax = 5.0f;
    Color startColor = {255, 255, 255, 255};
    float angularVelocityMin = -90.0f;
    float angularVelocityMax = 90.0f;
    float gravityScaleMin = 0.8f;
    float gravityScaleMax = 1.2f;
    float targetGroundY = -1.0f;
    ParticleProperties() = default;
};


class ParticleSystem
{
public:
    ParticleSystem(int maxParticlesCount);
    ~ParticleSystem();

    void Update(float deltaTime);
    void Draw() const;


    void Emit(Vector2 emitterPosition, int count, const ParticleProperties& props, float worldScrollSpeedX = 0.0f);

    void SetGravity(Vector2 newGravity);
    void Reset();
    int GetActiveParticlesCount() const;

private:
    std::vector<Particle> particlesPool;
    int poolIndex;
    Vector2 systemGravity;
};

#endif
