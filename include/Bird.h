#ifndef BIRD_H
#define BIRD_H

#include "raylib.h"
#include <vector>

class Bird
{
public:
    Bird(float startX, float startY, float birdSpeed, const std::vector<Texture2D>& frames);
    ~Bird();

    void Update(float deltaTime);
    void Draw() const;

    Rectangle GetCollisionRect() const;
    bool IsOffScreen() const;
    float GetWidth() const;
    float GetHeight() const;
    void setSpeed(float newSpeed);
    Vector2 getPosition() const { return position; }

private:
    Vector2 position;
    float speed;
    Rectangle collisionRect;
    std::vector<Texture2D> flyFrames;
    int currentFrame;
    float frameTimeCounter;
    float frameSpeed;
    void UpdateCollisionRect();
};

#endif // BIRD_H
