#ifndef BIRD_H
#define BIRD_H

#include "raylib.h"
#include <vector>

class Bird
{
public:
    Vector2 position;
    float speed; // 由 Game 的 gameSpeed 设定
    Rectangle collisionRect;

    std::vector<Texture2D> flyFrames;
    int currentFrame;
    float frameTimeCounter;
    float frameSpeed;

    Bird(float startX, float startY, float birdSpeed, const std::vector<Texture2D>& frames);
    ~Bird();

    void Update(float deltaTime);
    void Draw();
    Rectangle GetCollisionRect() const;
    bool IsOffScreen() const;
    float GetWidth() const;
    float GetHeight() const;

private:
    void UpdateCollisionRect();
};

#endif // BIRD_H
