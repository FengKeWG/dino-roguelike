// include/Bird.h
#ifndef BIRD_H
#define BIRD_H

#include "raylib.h"
#include <vector>

class Bird
{
public:
    Bird(float startX, float startY, float birdSpeed, const std::vector<Texture2D>& frames);
    ~Bird();

    // 更新鸟的状态
    void Update(float deltaTime);
    // 绘制鸟
    void Draw() const;

    // 获取碰撞矩形
    Rectangle GetCollisionRect() const;
    // 检查鸟是否移出屏幕
    bool IsOffScreen() const;
    // 获取鸟的宽度
    float GetWidth() const;
    // 获取鸟的高度
    float GetHeight() const;
    // 设置鸟的速度
    void setSpeed(float newSpeed);
    // 获取鸟的位置
    Vector2 getPosition() const { return position; }

private:
    Vector2 position; // 鸟的位置
    float speed; // 鸟的飞行速度
    Rectangle collisionRect; // 碰撞检测矩形
    std::vector<Texture2D> flyFrames; // 飞行状态的动画帧
    int currentFrame; // 当前动画帧索引
    float frameTimeCounter; // 帧动画计时器
    float frameSpeed; // 动画播放速度
    // 更新碰撞矩形位置
    void UpdateCollisionRect();
};

#endif // BIRD_H
