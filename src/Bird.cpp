// src/Bird.cpp
#include "../include/Bird.h"

Bird::Bird(const float startX, const float startY, const float birdSpeed, const std::vector<Texture2D>& frames)
    : position({startX, startY}),
      speed(birdSpeed),
      collisionRect({startX, startY, 30, 20}),
      flyFrames(frames),
      currentFrame(0),
      frameTimeCounter(0.0f),
      frameSpeed(0.15f)
{
    collisionRect = {
        position.x,
        position.y,
        static_cast<float>(flyFrames[0].width),
        static_cast<float>(flyFrames[0].height)
    };
}

Bird::~Bird() = default;

// 更新鸟的状态，每帧调用
void Bird::Update(const float deltaTime)
{
    // 根据速度和时间差更新鸟的X轴位置 (向左移动)
    position.x -= speed * deltaTime;

    // 更新动画帧
    frameTimeCounter += deltaTime;
    if (frameTimeCounter >= frameSpeed)
    {
        frameTimeCounter = 0.0f; // 重置计时器
        currentFrame++; // 切换到下一帧
        if (currentFrame >= flyFrames.size()) // 如果超出最后一帧
        {
            currentFrame = 0; // 回到第一帧，实现循环动画
        }
    }
    // 更新碰撞矩形的位置和大小
    UpdateCollisionRect();
}

// 绘制鸟
void Bird::Draw() const
{
    // 绘制当前动画帧的纹理
    DrawTexture(flyFrames[currentFrame], static_cast<int>(position.x), static_cast<int>(position.y), WHITE);
}

// 更新碰撞矩形的位置和大小，使其与鸟的当前状态同步
void Bird::UpdateCollisionRect()
{
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    collisionRect.width = GetWidth(); // 使用GetWidth获取当前帧的宽度
    collisionRect.height = GetHeight(); // 使用GetHeight获取当前帧的高度
}

// 获取鸟的碰撞矩形
Rectangle Bird::GetCollisionRect() const
{
    return collisionRect;
}

// 检查鸟是否完全移出屏幕左侧
bool Bird::IsOffScreen() const
{
    // 如果鸟的右边缘小于0 (屏幕最左侧)，则认为其移出屏幕
    return (position.x + GetWidth()) < 0;
}

// 获取鸟的当前宽度
float Bird::GetWidth() const
{
    return static_cast<float>(flyFrames[currentFrame].width);
}

// 获取鸟的当前高度
float Bird::GetHeight() const
{
    return static_cast<float>(flyFrames[currentFrame].height);
}

// 设置鸟的飞行速度
void Bird::setSpeed(const float newSpeed)
{
    speed = newSpeed;
}
