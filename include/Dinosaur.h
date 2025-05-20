#ifndef DINOSAUR_H
#define DINOSAUR_H

#include "raylib.h"
#include "Utils.h"
#include <vector>
#include "ParticleSystem.h"

class Dinosaur
{
public:
    Vector2 position;
    Vector2 velocity;
    float groundY;
    float runHeight;
    float sneakHeight;

    Dinosaur(float startX, float initialGroundY,
             const std::vector<Texture2D>& runTex,
             const std::vector<Texture2D>& sneakTex,
             const Texture2D& deadTex,
             const Sound& jumpSound,
             const Sound& dashSound);
    ~Dinosaur();

    void Update(float deltaTime, float worldScrollSpeed);
    void Draw() const;

    void RequestJump();
    void StartSneaking();

    void StopSneaking();
    void Move(float direction, float deltaTime);
    void RequestDash();
    void MarkAsDead();

    Rectangle GetCollisionRect() const;
    float GetWidth() const;
    float GetHeight() const;
    bool IsOnGround() const;
    bool IsSneaking() const { return isSneaking; }
    bool IsJumping() const { return isJumping; }
    bool IsFacingRight() const { return facingRight; }

    void UpdateCollisionRect();

private:
    Sound jumpSoundHandle;
    Sound dashSoundHandle;
    bool isJumping;
    bool isSneaking;
    bool facingRight;

    std::vector<Texture2D> runFrames;
    std::vector<Texture2D> sneakFrames;
    Texture2D deadTexture;
    bool isDead;
    int currentAnimFrameIndex;
    float frameTimeCounter;
    float animationSpeed;

    Rectangle collisionRect;

    float gravity;
    float jumpSpeed;
    float coyoteTimeDuration = 0.1f;
    float coyoteTimeCounter;
    float jumpBufferDuration = 0.1f;
    float jumpBufferCounter;
    bool jumpQueued;
    float sneakGravityMultiplier;
    float moveSpeed;

    bool isDashing;
    float dashSpeedMagnitude;
    float dashDuration;
    float dashTimer;
    float dashCooldown;
    float dashCooldownTimer;
    Vector2 dashDirection;

    ParticleSystem dashTrailParticles;
    ParticleProperties dashParticleProps;

    void ExecuteJump();
    const std::vector<Texture2D>* GetCurrentAnimationFramesPointer() const;
    Texture2D GetCurrentTextureToDraw() const;
};

#endif // DINOSAUR_H
