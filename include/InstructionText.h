// include/InstructionText.h
#ifndef INSTRUCTION_TEXT_H
#define INSTRUCTION_TEXT_H

#include "raylib.h"
#include "Utils.h" // 工具函数
#include <string>
#include <vector>
#include "ParticleSystem.h" // 包含粒子系统

// 教学文本状态枚举
enum class InstructionTextState
{
    INACTIVE, // 未激活
    DISPLAYING, // 显示中
    FALLING, // 掉落中
    EXPLODING, // 爆炸中 (粒子效果)
    DONE // 完成
};

// 管理单个教学文本的显示和行为
class InstructionText
{
public:
    InstructionText();

    // 初始化教学文本
    void Initialize(const char* text,
                    int fontSize,
                    Color textColor,
                    float displayDuration,
                    float fallGravity,
                    int virtualScreenWidth,
                    float groundY,
                    const Sound& explosionSfx);

    // 激活教学文本，使其开始显示
    void Activate(Vector2 startPos);
    // 更新教学文本状态 (如计时、掉落、爆炸)
    void Update(float deltaTime, float worldScrollSpeed);
    // 绘制教学文本
    void Draw() const;
    // 重置教学文本状态
    void Reset();
    // 检查教学文本是否已完成其生命周期
    bool IsDone() const;
    // 检查教学文本当前是否处于激活状态 (非INACTIVE或DONE)
    bool IsActive() const;
    // 获取文本的碰撞矩形
    Rectangle GetCollisionRect() const { return textBounds; }
    // 获取当前状态
    InstructionTextState GetCurrentState() const { return currentState; }

private:
    InstructionTextState currentState; // 当前状态
    std::string message; // 显示的文本信息
    int fontSize; // 字体大小
    Color textColor; // 文本颜色
    Rectangle textBounds; // 文本边界矩形
    Vector2 textDrawPosition; // 文本绘制的左上角位置
    Vector2 fallVelocity; // 掉落速度
    float displayTime; // 文本固定显示时长
    float currentTimer; // 当前计时器
    float gravity; // 掉落时的重力加速度
    float groundReferenceY; // 地面Y坐标参考 (用于停止掉落)
    Sound bombSound; // 爆炸音效

    ParticleSystem explosionParticles; // 爆炸粒子效果
    ParticleProperties explosionParticleProps; // 爆炸粒子属性
    float explosionDuration; // 爆炸效果持续时间
    int screenWidthForCentering; // 用于居中计算的屏幕宽度

    // 计算文本的布局和初始绘制位置
    void CalculateTextLayout(Vector2 startPos = {-1, -1});
};

#endif // INSTRUCTION_TEXT_H
