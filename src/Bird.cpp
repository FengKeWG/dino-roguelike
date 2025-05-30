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

void Bird::Update(const float deltaTime)
{
    position.x -= speed * deltaTime;
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
        DrawRectangleRec(GetCollisionRect(), SKYBLUE);
    }
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
    return 30.0f;
}

float Bird::GetHeight() const
{
    if (!flyFrames.empty() && flyFrames[currentFrame].id > 0)
    {
        return static_cast<float>(flyFrames[currentFrame].height);
    }
    return 20.0f;
}

void Bird::setSpeed(const float newSpeed)
{
    speed = newSpeed;
}
