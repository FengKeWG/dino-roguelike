// include/ParticleSystem.h
#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "raylib.h"
#include "Utils.h"
#include <vector>
#include <string>

// 单个粒子
struct Particle
{
    Vector2 position = {0, 0}; // 粒子位置
    Vector2 velocity = {0, 0}; // 粒子速度
    Color color = WHITE; // 粒子颜色
    float size = 1.0f; // 粒子大小
    float lifeTime = 1.0f; // 粒子总生命周期
    float lifeRemaining = 0.0f; // 粒子剩余生命
    float rotation = 0.0f; // 粒子旋转角度
    float angularVelocity = 0.0f; // 粒子角速度
    float gravityEffect = 1.0f; // 受重力影响的程度
    bool isActive = false; // 粒子是否激活
    bool isOnGround = false; // 粒子是否在地面上
    float groundYLevel = 0.0f; // 地面Y坐标
    float groundScrollSpeedX = 0.0f; // 地面滚动速度
};

// 粒子发射时的属性
struct ParticleProperties
{
    float lifeTimeMin = 0.5f; // 最小生命周期
    float lifeTimeMax = 1.5f; // 最大生命周期
    float initialSpeedMin = 50.0f; // 最小初始速度
    float initialSpeedMax = 150.0f; // 最大初始速度
    float emissionAngleMin = 0.0f; // 最小发射角度
    float emissionAngleMax = 360.0f; // 最大发射角度
    float startSizeMin = 2.0f; // 最小初始大小
    float startSizeMax = 5.0f; // 最大初始大小
    Color startColor = {255, 255, 255, 255}; // 初始颜色
    float angularVelocityMin = -90.0f; // 最小角速度
    float angularVelocityMax = 90.0f; // 最大角速度
    float gravityScaleMin = 0.8f; // 最小重力缩放
    float gravityScaleMax = 1.2f; // 最大重力缩放
    float targetGroundY = -1.0f; // 目标地面Y坐标
};

// 粒子系统管理器
class ParticleSystem
{
public:
    ParticleSystem(int maxParticlesCount);
    ~ParticleSystem();

    // 更新所有激活粒子的状态
    void Update(float deltaTime);
    // 绘制所有激活粒子
    void Draw() const;

    // 从指定位置发射指定数量的粒子
    void Emit(Vector2 emitterPosition, int count, const ParticleProperties& props, float worldScrollSpeedX = 0.0f);

    // 设置整个粒子系统的重力向量
    void SetGravity(Vector2 newGravity);
    // 重置粒子池，使所有粒子变为非激活状态
    void Reset();
    // 获取当前激活的粒子数量
    int GetActiveParticlesCount() const;

private:
    std::vector<Particle> particlesPool; // 粒子对象池
    int poolIndex; // 对象池当前索引，用于循环使用粒子
    Vector2 systemGravity; // 粒子系统应用的重力
};

#endif // PARTICLE_SYSTEM_H
