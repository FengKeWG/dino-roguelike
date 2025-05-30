// include/Obstacle.h
#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "raylib.h"

class Obstacle
{
public:
    Obstacle(float startX, float groundY, float obstacleSpeed, const Texture2D& tex);
    ~Obstacle();

    // 更新障碍物状态
    void Update(float deltaTime);
    // 绘制障碍物
    void Draw() const;

    // 获取碰撞矩形
    Rectangle GetCollisionRect() const;
    // 检查障碍物是否移出屏幕
    bool IsOffScreen() const;
    // 获取障碍物宽度
    float GetWidth() const;
    // 获取障碍物当前速度
    float getSpeed() const;
    // 设置障碍物速度
    void setSpeed(float newSpeed);

private:
    Vector2 position; // 障碍物位置
    float speed; // 障碍物移动速度
    Rectangle collisionRect; // 碰撞检测矩形
    Texture2D texture; // 障碍物纹理
    // 更新碰撞矩形位置
    void UpdateCollisionRect();
};

#endif // OBSTACLE_H
