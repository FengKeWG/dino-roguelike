#ifndef DINOSAUR_H
#define DINOSAUR_H

#include "raylib.h"
#include <vector>

class Dinosaur {
public:
    Vector2 position;
    Vector2 velocity;
    float gravity;
    float jumpSpeed;
    bool isJumping;
    Rectangle collisionRect;

    // 动画相关
    std::vector<Texture2D> runFrames;
    Texture2D jumpFrame;
    int currentRunFrameIndex;
    float frameTimeCounter;
    float frameSpeed; // 每帧显示时间 (秒)

    float groundY; // 脚应该站立的地面Y坐标

    Dinosaur(float startX, float initialGroundY, const std::vector<Texture2D> &frames, Texture2D jumpTex);

    ~Dinosaur();

    void Jump();

    void Update(float deltaTime);

    void Draw();

    Rectangle GetCollisionRect() const;

    float GetWidth() const;

    float GetHeight() const;

    void UpdateCollisionRect();

private:
    Texture2D GetCurrentTexture() const;
};

#endif
