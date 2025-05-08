#include "../include/Bird.h"
#include <iostream>

Bird::Bird(const float startX, const float startY, const float birdSpeed, const std::vector<Texture2D>& frames)
    : speed(birdSpeed), flyFrames(frames), currentFrame(0),
      frameTimeCounter(0.0f), frameSpeed(0.15f)
{
    // 小鸟动画帧速

    if (flyFrames.empty() || flyFrames[0].id == 0)
    {
        std::cerr << "Warning: Bird frames not loaded or empty!" << std::endl;
        position = {startX, startY};
        collisionRect = {position.x, position.y, 30, 20}; // 默认碰撞盒
    }
    else
    {
        position = {startX, startY};
        collisionRect = {
            position.x,
            position.y,
            static_cast<float>(flyFrames[0].width),
            static_cast<float>(flyFrames[0].height)
        };
    }
}

Bird::~Bird() = default;

void Bird::Update(const float deltaTime)
{
    position.x -= speed * deltaTime;

    // 动画更新
    frameTimeCounter += deltaTime;
    if (frameTimeCounter >= frameSpeed)
    {
        frameTimeCounter = 0.0f;
        currentFrame++;
        if (currentFrame >= flyFrames.size())
        {
            currentFrame = 0;
        }
    }
    UpdateCollisionRect();
}

void Bird::Draw() const
{
    if (!flyFrames.empty() && flyFrames[currentFrame].id > 0)
    {
        DrawTexture(flyFrames[currentFrame], static_cast<int>(position.x), static_cast<int>(position.y), WHITE);
    }
    else
    {
        DrawRectangleRec(GetCollisionRect(), SKYBLUE); // 备用绘制
    }
    // DrawRectangleLinesEx(GetCollisionRect(), 1, DARKBLUE);
}

void Bird::UpdateCollisionRect()
{
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    collisionRect.width = GetWidth();
    collisionRect.height = GetHeight();
}

Rectangle Bird::GetCollisionRect() const
{
    return collisionRect;
}

bool Bird::IsOffScreen() const
{
    return (position.x + GetWidth()) < 0;
}

float Bird::GetWidth() const
{
    if (!flyFrames.empty() && flyFrames[currentFrame].id > 0)
    {
        return static_cast<float>(flyFrames[currentFrame].width);
    }
    return 30.0f; // Default
}

float Bird::GetHeight() const
{
    if (!flyFrames.empty() && flyFrames[currentFrame].id > 0)
    {
        return static_cast<float>(flyFrames[currentFrame].height);
    }
    return 20.0f; // Default
}
