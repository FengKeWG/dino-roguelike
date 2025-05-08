#include "../include/Dinosaur.h"
#include <iostream>

Dinosaur::Dinosaur(float startX, float initialGroundY, const std::vector<Texture2D>& frames, Texture2D jumpTex)
    : velocity({0, 0}), gravity(1200.0f), jumpSpeed(-500.0f),
      isJumping(false), runFrames(frames), jumpFrame(jumpTex), currentRunFrameIndex(0),
      frameTimeCounter(0.0f), frameSpeed(0.1f), // 每0.1秒换一帧
      groundY(initialGroundY)
{
    Texture2D initialTexture = GetCurrentTexture(); // 获取初始纹理（通常是跑动第一帧）

    if (initialTexture.id == 0)
    {
        std::cerr << "Warning: Dinosaur initial texture not loaded!" << std::endl;
        position = {startX, initialGroundY - 50.0f};
        collisionRect = {position.x, position.y, 30, 50};
    }
    else
    {
        position = {startX, initialGroundY - static_cast<float>(initialTexture.height)};
        collisionRect = {
            position.x,
            position.y,
            static_cast<float>(initialTexture.width),
            static_cast<float>(initialTexture.height)
        };
    }
}

Dinosaur::~Dinosaur()
{
    // 纹理的加载和卸载由 Game 类管理
}

Texture2D Dinosaur::GetCurrentTexture() const
{
    if (isJumping && jumpFrame.id > 0)
    {
        return jumpFrame;
    }
    if (!runFrames.empty() && runFrames[currentRunFrameIndex].id > 0)
    {
        return runFrames[currentRunFrameIndex];
    }
    // 返回一个空的或默认的 Texture2D，如果都没有加载成功
    // 或者在构造函数中确保至少有一个有效纹理或抛出错误
    return Texture2D{0}; // 表示无效纹理
}

void Dinosaur::Jump()
{
    if (!isJumping)
    {
        velocity.y = jumpSpeed;
        isJumping = true;
        // TODO: 可以添加跳跃特定帧或状态
    }
}

void Dinosaur::Update(float deltaTime)
{
    position.y += velocity.y * deltaTime;
    velocity.y += gravity * deltaTime;

    if (!isJumping)
    {
        // 只在地面上跑动时播放动画
        frameTimeCounter += deltaTime;
        if (frameTimeCounter >= frameSpeed)
        {
            frameTimeCounter = 0.0f;
            currentRunFrameIndex++;
            if (currentRunFrameIndex >= runFrames.size())
            {
                currentRunFrameIndex = 0;
            }
        }
    }
    // 跳跃时，GetCurrentTexture() 会自动选择 jumpFrame

    float currentDinoHeight = GetHeight();
    if (position.y + currentDinoHeight >= groundY)
    {
        position.y = groundY - currentDinoHeight;
        velocity.y = 0;
        if (isJumping)
        {
            // 刚刚落地
            isJumping = false;
            // 可以重置跑动动画到第一帧，如果需要
            // currentRunFrameIndex = 0;
        }
    }

    UpdateCollisionRect();
}

void Dinosaur::Draw()
{
    Texture2D texToDraw = GetCurrentTexture();
    if (texToDraw.id > 0)
    {
        DrawTexture(texToDraw, static_cast<int>(position.x), static_cast<int>(position.y), WHITE);
    }
    else
    {
        DrawRectangleRec(GetCollisionRect(), LIME);
    }
    // DrawRectangleLinesEx(GetCollisionRect(), 1, RED);
}

void Dinosaur::UpdateCollisionRect()
{
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    collisionRect.width = GetWidth();
    collisionRect.height = GetHeight();
}

Rectangle Dinosaur::GetCollisionRect() const
{
    // 可以稍微调整碰撞盒，使其更符合实际视觉效果，例如缩小一点
    // Rectangle adjustedRect = collisionRect;
    // adjustedRect.x += 5;
    // adjustedRect.width -= 10;
    // adjustedRect.y += 5;
    // adjustedRect.height -= 5;
    // return adjustedRect;
    return collisionRect;
}

float Dinosaur::GetWidth() const
{
    Texture2D tex = GetCurrentTexture();
    if (tex.id > 0)
    {
        return static_cast<float>(tex.width);
    }
    return 30.0f;
}

float Dinosaur::GetHeight() const
{
    Texture2D tex = GetCurrentTexture();
    if (tex.id > 0)
    {
        return static_cast<float>(tex.height);
    }
    return 50.0f;
}
