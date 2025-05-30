#include "../include/Obstacle.h"

Obstacle::Obstacle(const float startX, const float groundY, const float obstacleSpeed, const Texture2D& tex)
    : position({0.0f, 0.0f}), speed(obstacleSpeed), collisionRect({0.0f, 0.0f, 20.0f, 50.0f}), texture(tex)
{
    float obstacleHeight = 50.0f;
    float obstacleWidth = 20.0f;
    if (texture.id != 0)
    {
        obstacleHeight = static_cast<float>(texture.height);
        obstacleWidth = static_cast<float>(texture.width);
    }
    position = {startX, (groundY + 8.0f) - obstacleHeight};
    collisionRect = {
        position.x,
        position.y,
        obstacleWidth,
        obstacleHeight
    };
}

Obstacle::~Obstacle() = default;

void Obstacle::Update(const float deltaTime)
{
    position.x -= speed * deltaTime;
    UpdateCollisionRect();
}

void Obstacle::Draw() const
{
    if (texture.id > 0)
    {
        DrawTexture(texture, static_cast<int>(position.x), static_cast<int>(position.y), WHITE);
    }
    else
    {
        DrawRectangleRec(collisionRect, RED);
    }
}

void Obstacle::UpdateCollisionRect()
{
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    if (texture.id > 0)
    {
        collisionRect.width = static_cast<float>(texture.width);
        collisionRect.height = static_cast<float>(texture.height);
    }
}

Rectangle Obstacle::GetCollisionRect() const
{
    return collisionRect;
}

bool Obstacle::IsOffScreen() const
{
    return (position.x + GetWidth()) < 0;
}

float Obstacle::GetWidth() const
{
    if (texture.id > 0)
    {
        return static_cast<float>(texture.width);
    }
    return 20.0f;
}

float Obstacle::getSpeed() const
{
    return speed;
}

void Obstacle::setSpeed(const float newSpeed)
{
    speed = newSpeed;
}
