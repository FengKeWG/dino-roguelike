#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include "raylib.h"

class Obstacle {
public:
    Vector2 position;
    float speed; // 由Game的gameSpeed设定
    Rectangle collisionRect;
    Texture2D texture; // 由Game传入具体的仙人掌纹理

    // groundYPos 是障碍物底部应该对齐的地面Y坐标
    Obstacle(float startX, float groundYPos, float obstacleSpeed, Texture2D tex);

    ~Obstacle();

    void Update(float deltaTime);

    void Draw();

    Rectangle GetCollisionRect() const;

    bool IsOffScreen(float screenWidth) const; // 需要知道屏幕宽度
    float GetWidth() const;

    void UpdateCollisionRect();
};

#endif // OBSTACLE_H
