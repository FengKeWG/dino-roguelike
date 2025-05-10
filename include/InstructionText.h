// InstructionText.h
#ifndef INSTRUCTION_TEXT_H
#define INSTRUCTION_TEXT_H

#include "raylib.h"
#include <string>
#include <vector>
#include "ParticleSystem.h" // <--- 包含新的粒子系统

// ... (InstructionTextState enum 保持不变) ...
enum class InstructionTextState
{
    INACTIVE,
    DISPLAYING,
    FALLING,
    EXPLODING,
    DONE
};

// ---- REMOVE: InstructionTextParticle 结构体 ----
// struct InstructionTextParticle { ... };

class InstructionText
{
public:
    InstructionText();

    void Initialize(const char* text,
                    int fontSize,
                    Color textColor,
                    float displayDuration,
                    float fallGravity,
                    int virtualScreenWidth,
                    float groundY,
                    const Sound& explosionSfx); // <--- 添加 groundY 用于粒子

    void Activate(Vector2 startPos); // groundY 已在Initialize时传入
    void Update(float deltaTime);
    void Draw() const;
    void Reset();
    bool IsDone() const;
    bool IsActive() const;

private:
    InstructionTextState currentState;
    std::string message;
    int fontSize;
    Color textColor;
    Rectangle textBounds;
    Vector2 textDrawPosition;
    Vector2 fallVelocity;
    float displayTime;
    float currentTimer;
    float gravity;
    float groundReferenceY; // 文本框碰撞地面用
    Sound bombSound;
    // ---- REMOVE: 旧的粒子vector ----
    // std::vector<InstructionTextParticle> particles;
    // float explosionEffectTimer;
    // float currentExplosionTime;

    // ---- NEW: 粒子系统实例 ----
    ParticleSystem explosionParticles;
    ParticleProperties explosionParticleProps; // 存储爆炸粒子的配置
    float explosionDuration; // 爆炸效果总持续时间
    int screenWidthForCentering;

    // ---- REMOVE: 旧的粒子私有方法 ----
    // void SpawnExplosionParticles();
    // void UpdateExplosionParticles(float deltaTime);
    // void DrawExplosionParticles() const;

    void CalculateTextLayout(Vector2 startPos = {-1, -1});
};

#endif // INSTRUCTION_TEXT_H
