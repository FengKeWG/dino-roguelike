// src/InstructionText.cpp
#include "../include/InstructionText.h"

InstructionText::InstructionText()
    : currentState(InstructionTextState::INACTIVE),
      message(""), fontSize(20), textColor(BLACK),
      textBounds({0, 0, 0, 0}),
      textDrawPosition({0, 0}),
      fallVelocity({0, 0}),
      displayTime(2.0f), currentTimer(0.0f), gravity(1000.0f), groundReferenceY(0.0f),
      explosionParticles(300),
      explosionDuration(1.0f),
      screenWidthForCentering(960)
{
    // 配置爆炸粒子的默认属性
    explosionParticleProps.lifeTimeMin = 2.0f;
    explosionParticleProps.lifeTimeMax = 4.0f;
    explosionParticleProps.initialSpeedMin = 100.0f;
    explosionParticleProps.initialSpeedMax = 500.0f;
    explosionParticleProps.emissionAngleMin = 180.0f;
    explosionParticleProps.emissionAngleMax = 360.0f;
    explosionParticleProps.startSizeMin = 2.0f;
    explosionParticleProps.startSizeMax = 8.0f;
    explosionParticleProps.angularVelocityMin = -360.0f;
    explosionParticleProps.angularVelocityMax = 360.0f;
    explosionParticleProps.gravityScaleMin = 0.4f;
    explosionParticleProps.gravityScaleMax = 0.8f;
}

void InstructionText::Initialize(const char* text, int fs, Color tColor,
                                 float dispTime, float fallGrav, int virtualScreenWidth, float groundYVal,
                                 const Sound& explosionSfx)
{
    message = text;
    fontSize = fs;
    textColor = tColor;
    displayTime = dispTime;
    gravity = fallGrav;
    screenWidthForCentering = virtualScreenWidth;
    groundReferenceY = groundYVal;
    bombSound = explosionSfx;
    explosionParticleProps.startColor = textColor;
    explosionParticleProps.targetGroundY = groundReferenceY + 5.0f;
    explosionParticles.SetGravity({0, gravity * 0.8f});
    Reset();
}

// 计算文本的布局
void InstructionText::CalculateTextLayout(const Vector2 startPos)
{
    // 测量文本的宽度和高度
    const auto [textWidth, textHeight] = MeasureTextEx(GetFontDefault(), message.c_str(), static_cast<float>(fontSize),
                                                       1);
    textBounds.width = textWidth;
    textBounds.height = textHeight;
    textBounds.x = startPos.x - textBounds.width / 2.0f;
    textBounds.y = startPos.y - textBounds.height / 2.0f;
    textDrawPosition = {textBounds.x, textBounds.y}; // 更新文本的绘制位置
}

// 激活文本
void InstructionText::Activate(const Vector2 startPos)
{
    CalculateTextLayout(startPos); // 根据起始位置计算布局
    currentState = InstructionTextState::DISPLAYING; // 设置状态为显示中
    currentTimer = 0.0f; // 重置计时器
    fallVelocity = {0, 0}; // 重置掉落速度
    explosionParticles.Reset(); // 重置粒子系统
}

// 重置文本状态到初始未激活状态
void InstructionText::Reset()
{
    currentState = InstructionTextState::INACTIVE; // 设置为未激活
    CalculateTextLayout(); // 重新计算默认布局
    fallVelocity = {0, 0}; // 重置掉落速度
    currentTimer = 0.0f; // 重置计时器
    explosionParticles.Reset(); // 重置粒子系统
}

bool InstructionText::IsDone() const
{
    return currentState == InstructionTextState::DONE || currentState == InstructionTextState::INACTIVE;
}

bool InstructionText::IsActive() const
{
    return currentState != InstructionTextState::INACTIVE && currentState != InstructionTextState::DONE;
}

void InstructionText::Update(const float deltaTime, const float worldScrollSpeed)
{
    if (!IsActive() && currentState != InstructionTextState::EXPLODING)
    {
        return;
    }

    currentTimer += deltaTime; // 更新内部计时器

    // 根据当前状态执行不同逻辑
    switch (currentState)
    {
    case InstructionTextState::DISPLAYING: // 显示状态
        if (currentTimer >= displayTime) // 如果显示时间已到
        {
            currentState = InstructionTextState::FALLING; // 切换到掉落状态
            currentTimer = 0.0f; // 重置计时器用于下一阶段
        }
        break;
    case InstructionTextState::FALLING: // 掉落状态
        fallVelocity.y += gravity * deltaTime; // 应用重力到垂直速度
        textBounds.y += fallVelocity.y * deltaTime; // 更新文本Y位置
        textDrawPosition.y = textBounds.y; // 同步绘制位置

        // 如果文本底部接触或穿过地面
        if (textBounds.y + textBounds.height >= groundReferenceY + 5.0f)
        {
            textBounds.y = groundReferenceY - textBounds.height + 5.0f; // 精确放置在地面上
            textDrawPosition.y = textBounds.y;
            fallVelocity.y = 0; // 停止下落
            currentState = InstructionTextState::EXPLODING; // 切换到爆炸状态
            currentTimer = 0.0f; // 重置计时器
            PlaySound(bombSound);

            // 计算爆炸中心点
            const Vector2 explosionCenter = {
                textBounds.x + textBounds.width / 2.0f,
                textBounds.y + textBounds.height / 2.0f
            };
            // 发射爆炸粒子
            explosionParticles.Emit(explosionCenter, randI(80, 150), explosionParticleProps, worldScrollSpeed);
        }
        break;
    case InstructionTextState::EXPLODING: // 爆炸状态
        explosionParticles.Update(deltaTime); // 更新粒子效果
        if (currentTimer >= explosionDuration && explosionParticles.GetActiveParticlesCount() == 0)
        {
            currentState = InstructionTextState::DONE; // 切换到完成状态
        }
        break;
    case InstructionTextState::INACTIVE:
    case InstructionTextState::DONE: // 完成状态
        if (explosionParticles.GetActiveParticlesCount() > 0)
        {
            explosionParticles.Update(deltaTime);
        }
        break;
    }
}

void InstructionText::Draw() const
{
    if (currentState == InstructionTextState::DISPLAYING || currentState == InstructionTextState::FALLING)
    {
        DrawText(message.c_str(), static_cast<int>(textDrawPosition.x), static_cast<int>(textDrawPosition.y),
                 fontSize,
                 textColor);
    }
    if (explosionParticles.GetActiveParticlesCount() > 0)
    {
        explosionParticles.Draw();
    }
}
