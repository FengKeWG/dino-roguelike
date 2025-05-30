#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "raylib.h"

class Obstacle
{
public:
    Obstacle(float startX, float groundY, float obstacleSpeed, const Texture2D& tex);
    ~Obstacle();

    void Update(float deltaTime);
    void Draw() const;

    Rectangle GetCollisionRect() const;
    bool IsOffScreen() const;
    float GetWidth() const;
    float getSpeed() const;
    void setSpeed(float newSpeed);

private:
    Vector2 position;
    float speed;
    Rectangle collisionRect;
    Texture2D texture;
    void UpdateCollisionRect();
};

#endif // OBSTACLE_H
