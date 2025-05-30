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

void InstructionText::CalculateTextLayout(const Vector2 startPos)
{
    const auto [x, y] = MeasureTextEx(GetFontDefault(), message.c_str(), static_cast<float>(fontSize), 1);
    textBounds.width = x;
    textBounds.height = y;
    if (startPos.x < 0)
    {
        textBounds.x = (screenWidthForCentering - textBounds.width) / 2.0f;
        textBounds.y = 60;
    }
    else
    {
        textBounds.x = startPos.x - textBounds.width / 2.0f;
        textBounds.y = startPos.y - textBounds.height / 2.0f;
    }
    textDrawPosition = {textBounds.x, textBounds.y};
}

void InstructionText::Activate(const Vector2 startPos)
{
    CalculateTextLayout(startPos);
    currentState = InstructionTextState::DISPLAYING;
    currentTimer = 0.0f;
    fallVelocity = {0, 0};
    explosionParticles.Reset();
}

void InstructionText::Reset()
{
    currentState = InstructionTextState::INACTIVE;
    CalculateTextLayout();
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

void InstructionText::Update(float deltaTime, float worldScrollSpeed)
{
    if (!IsActive() && currentState != InstructionTextState::EXPLODING)
    {
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
        textBounds.y += fallVelocity.y * deltaTime;
        textDrawPosition.y = textBounds.y;
        if (textBounds.y + textBounds.height >= groundReferenceY + 5.0f)
        {
            textBounds.y = groundReferenceY - textBounds.height + 5.0f;
            textDrawPosition.y = textBounds.y;
            fallVelocity.y = 0;
            currentState = InstructionTextState::EXPLODING;
            currentTimer = 0.0f;
            PlaySound(bombSound);
            const Vector2 explosionCenter = {
                textBounds.x + textBounds.width / 2.0f,
                textBounds.y + textBounds.height / 2.0f
            };
            explosionParticles.Emit(explosionCenter, randI(80, 150), explosionParticleProps, worldScrollSpeed);
        }
        break;
    case InstructionTextState::EXPLODING:
        explosionParticles.Update(deltaTime);
        if (currentTimer >= explosionDuration && explosionParticles.GetActiveParticlesCount() == 0)
        {
            currentState = InstructionTextState::DONE;
        }
        break;
    case InstructionTextState::INACTIVE:
    case InstructionTextState::DONE:
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
