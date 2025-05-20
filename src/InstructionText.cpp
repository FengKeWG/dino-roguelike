// InstructionText.cpp
#include "../include/InstructionText.h"

InstructionText::InstructionText()
    : currentState(InstructionTextState::INACTIVE),
      message(""), fontSize(20), textColor(BLACK),
      textBounds({0, 0, 0, 0}), // <--- 修改
      textDrawPosition({0, 0}), // <--- 新增或调整
      fallVelocity({0, 0}), // <--- 修改
      displayTime(2.0f), currentTimer(0.0f), gravity(1000.0f), groundReferenceY(0.0f),
      explosionParticles(300),
      explosionDuration(1.0f),
      screenWidthForCentering(960)
{
    // ---- 配置爆炸粒子属性 ----
    explosionParticleProps.lifeTimeMin = 2.0f;
    explosionParticleProps.lifeTimeMax = 4.0f;
    explosionParticleProps.initialSpeedMin = 100.0f;
    explosionParticleProps.initialSpeedMax = 500.0f;
    explosionParticleProps.emissionAngleMin = 180.0f; // 270向上，180向右
    explosionParticleProps.emissionAngleMax = 360.0f;
    explosionParticleProps.startSizeMin = 2.0f;
    explosionParticleProps.startSizeMax = 8.0f;
    // 颜色可以基于文本框颜色
    explosionParticleProps.angularVelocityMin = -360.0f;
    explosionParticleProps.angularVelocityMax = 360.0f;
    explosionParticleProps.gravityScaleMin = 0.4f; // 受一些重力影响
    explosionParticleProps.gravityScaleMax = 0.8f;
    // explosionParticleProps.stopAtGroundY = groundReferenceY; // 粒子在地表停止 (需要动态设置)
    // explosionParticleProps.dieOnGroundContact = true;
    //explosionParticles.SetGravity({0, gravity * 0.7f}); // 粒子系统本身的重力
}

void InstructionText::Initialize(const char* text, int fs, Color tColor, /*Color bgColor,*/
                                 float dispTime, float fallGrav, int virtualScreenWidth, float groundYVal,
                                 const Sound& explosionSfx)
{
    message = text; // 英文文本将从 Game.cpp 传入
    fontSize = fs;
    textColor = tColor;
    // boxBgColor = bgColor; // <--- 移除
    displayTime = dispTime;
    gravity = fallGrav;
    screenWidthForCentering = virtualScreenWidth;
    groundReferenceY = groundYVal;
    bombSound = explosionSfx;

    explosionParticleProps.startColor = textColor;
    explosionParticleProps.targetGroundY = groundReferenceY + 5.0f;
    explosionParticles.SetGravity({0, gravity * 0.8f}); // 或者一个固定的值

    Reset(); // Reset 会调用 CalculateTextLayout
}

void InstructionText::CalculateTextLayout(const Vector2 startPos) // 重命名
{
    const auto [x, y] = MeasureTextEx(GetFontDefault(), message.c_str(), static_cast<float>(fontSize), 1);

    textBounds.width = x;
    textBounds.height = y;

    if (startPos.x < 0)
    {
        // 如果没有提供起始位置（例如在Reset时），则默认居中于屏幕上方
        textBounds.x = (screenWidthForCentering - textBounds.width) / 2.0f;
        textBounds.y = 60; // 初始Y位置 (可以调整)
    }
    else
    {
        // 如果提供了起始位置 (例如在Activate时，startPos是期望的中心点)
        textBounds.x = startPos.x - textBounds.width / 2.0f;
        textBounds.y = startPos.y - textBounds.height / 2.0f; // 让startPos成为文本的中心
    }
    textDrawPosition = {textBounds.x, textBounds.y};
}

void InstructionText::Activate(const Vector2 startPos) // 参数名修改以更清晰
{
    CalculateTextLayout(startPos); // desiredCenterPos 是文本的中心
    currentState = InstructionTextState::DISPLAYING;
    currentTimer = 0.0f;
    fallVelocity = {0, 0};
    explosionParticles.Reset();
}

void InstructionText::Reset()
{
    currentState = InstructionTextState::INACTIVE;
    CalculateTextLayout(); // 不传参数，使用默认屏幕上方居中位置
    fallVelocity = {0, 0};
    currentTimer = 0.0f;
    explosionParticles.Reset();
}

bool InstructionText::IsDone() const
{
    return currentState == InstructionTextState::DONE || currentState == InstructionTextState::INACTIVE;
}

bool InstructionText::IsActive() const
{
    return currentState != InstructionTextState::INACTIVE && currentState != InstructionTextState::DONE;
}

void InstructionText::Update(float deltaTime, float worldScrollSpeed) // <--- 新代码
{
    if (!IsActive() && currentState != InstructionTextState::EXPLODING)
    {
        if (currentState == InstructionTextState::EXPLODING && explosionParticles.GetActiveParticlesCount() == 0)
        {
            currentState = InstructionTextState::DONE;
        }
        return;
    }
    currentTimer += deltaTime;

    switch (currentState)
    {
    case InstructionTextState::DISPLAYING:
        if (currentTimer >= displayTime)
        {
            currentState = InstructionTextState::FALLING;
            currentTimer = 0.0f;
        }
        break;

    case InstructionTextState::FALLING:
        fallVelocity.y += gravity * deltaTime;
        textBounds.y += fallVelocity.y * deltaTime; // 更新文本边界的Y
        textDrawPosition.y = textBounds.y; // 绘制位置也跟随

        if (textBounds.y + textBounds.height >= groundReferenceY + 5.0f)
        {
            textBounds.y = groundReferenceY - textBounds.height + 5.0f; // 精确停在地面
            textDrawPosition.y = textBounds.y;
            fallVelocity.y = 0;
            currentState = InstructionTextState::EXPLODING;
            currentTimer = 0.0f;
            TraceLog(LOG_INFO, "InstructionText: Attempting to play bomb sound."); // <--- 日志1
            TraceLog(LOG_INFO, "InstructionText: bombSound.frameCount = %d", bombSound.frameCount); // <--- 日志2
            TraceLog(LOG_INFO, "InstructionText: IsAudioDeviceReady() = %s",
                     IsAudioDeviceReady() ? "true" : "false"); // <--- 日志3
            PlaySound(bombSound);
            TraceLog(LOG_INFO, "InstructionText: PlaySound(bombSound) CALLED.");
            const Vector2 explosionCenter = {
                textBounds.x + textBounds.width / 2.0f,
                textBounds.y + textBounds.height / 2.0f // 从文本区域中心爆炸
            };
            explosionParticles.Emit(explosionCenter, randI(80, 150), explosionParticleProps, worldScrollSpeed);
        }
        break;

    case InstructionTextState::EXPLODING:
        explosionParticles.Update(deltaTime);
        if (currentTimer >= explosionDuration && explosionParticles.GetActiveParticlesCount() == 0) // 爆炸时间到且粒子消失
        {
            currentState = InstructionTextState::DONE;
        }
        break;
    case InstructionTextState::INACTIVE:
    case InstructionTextState::DONE:
        // 如果是DONE，但仍有粒子，继续更新粒子
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
        // DrawRectangleRec(boxRec, boxBgColor); // <--- 移除绘制方框
        DrawText(message.c_str(), static_cast<int>(textDrawPosition.x), static_cast<int>(textDrawPosition.y),
                 fontSize,
                 textColor);
    }

    if (explosionParticles.GetActiveParticlesCount() > 0)
    {
        explosionParticles.Draw();
    }
}

// ---- REMOVE: SpawnExplosionParticles, UpdateExplosionParticles, DrawExplosionParticles ----
