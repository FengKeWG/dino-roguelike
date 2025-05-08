#ifndef DINOSAUR_H
#define DINOSAUR_H

#include "raylib.h"
#include <vector>

class Dinosaur
{
public:
    Vector2 position;
    Vector2 velocity;
    float gravity;
    float jumpSpeed;
    float moveSpeed;

    Sound jumpSoundHandle;

    bool isJumping;
    bool isSneaking;
    bool facingRight;

    Rectangle collisionRect;

    std::vector<Texture2D> runFrames;
    std::vector<Texture2D> sneakFrames;

    int currentAnimFrameIndex;
    float frameTimeCounter;
    float animationSpeed;

    float groundY;
    float runHeight;
    float sneakHeight;

    float coyoteTimeCounter;
    const float coyoteTimeDuration = 0.1f;

    float jumpBufferCounter;
    const float jumpBufferDuration = 0.12f;
    bool jumpQueued;

    // --- 新增：下蹲时的重力乘数 ---
    float sneakGravityMultiplier;

    Dinosaur(float startX, float initialGroundY,
             const std::vector<Texture2D>& runTex,
             const std::vector<Texture2D>& sneakTex,
             const Sound& jumpSound);
    ~Dinosaur();

    void RequestJump();
    void StartSneaking();
    void StopSneaking();
    void Move(float direction, float deltaTime);
    void Update(float deltaTime);
    void Draw() const;

    Rectangle GetCollisionRect() const;
    float GetWidth() const;
    float GetHeight() const;
    void UpdateCollisionRect();
    bool IsOnGround() const;

private:
    void ExecuteJump();
    Texture2D GetCurrentTextureToDraw() const;
    const std::vector<Texture2D>* GetCurrentAnimationFramesPointer() const;
};

#endif // DINOSAUR_H
