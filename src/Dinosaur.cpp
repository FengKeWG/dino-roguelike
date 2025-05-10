#include "../include/Dinosaur.h" // 确保路径正确
#include <iostream>
#include <cmath>
#include "raylib.h"
#include "raymath.h"

constexpr float GROUND_VISUAL_OFFSET = 5.0f;

Dinosaur::Dinosaur(const float startX, const float initialGroundY,
                   const std::vector<Texture2D>& runTex,
                   const std::vector<Texture2D>& sneakTex,
                   const Texture2D& deadTex,
                   const Sound& jumpSound,
                   const Sound& dashSound)
    : position({0, 0}), velocity({0, 0}), groundY(initialGroundY), runHeight(0.0f),
      sneakHeight(0.0f), jumpSoundHandle(jumpSound), dashSoundHandle(dashSound),
      isJumping(false), isSneaking(false), facingRight(true),
      runFrames(runTex), sneakFrames(sneakTex), deadTexture(deadTex),
      isDead(false), currentAnimFrameIndex(0), frameTimeCounter(0.0f),
      animationSpeed(0.08f), collisionRect({0, 0, 0, 0}),
      gravity(1800.0f), jumpSpeed(-600.0f), coyoteTimeCounter(0.0f),
      jumpBufferCounter(0.0f), jumpQueued(false),
      sneakGravityMultiplier(2.5f),
      moveSpeed(280.0f),
      isDashing(false),
      dashSpeedMagnitude(800.0f),
      dashDuration(0.18f),
      dashTimer(0.0f),
      dashCooldown(0.5f),
      dashCooldownTimer(0.0f),
      dashDirection({0.0f, 0.0f}),
      dashTrailParticles(150)
{
    runHeight = static_cast<float>(runFrames[0].height);
    sneakHeight = static_cast<float>(sneakFrames[0].height);
    position = {startX, initialGroundY - runHeight};
    UpdateCollisionRect();
    dashParticleProps.lifeTimeMin = 2.0f; // 粒子生命长一点，以便看到它们落地
    dashParticleProps.lifeTimeMax = 5.0f;
    dashParticleProps.initialSpeedMin = 30.0f;
    dashParticleProps.initialSpeedMax = 100.0f;
    // 发射角度会根据冲刺方向动态调整
    dashParticleProps.startSizeMin = 2.0f;
    dashParticleProps.startSizeMax = 4.0f;
    dashParticleProps.startColor = {80, 80, 80, 200}; // 深灰色，alpha不完全不透明
    dashParticleProps.angularVelocityMin = -220.0f;
    dashParticleProps.angularVelocityMax = 220.0f;
    dashParticleProps.gravityScaleMin = 0.1f; // 受重力影响更明显
    dashParticleProps.gravityScaleMax = 0.5f;
    // dashParticleProps.fadeOut = false; // 这个属性已移除

    // --- 关键: 设置粒子落地的Y坐标 ---
    // groundY 是恐龙脚底的视觉位置，粒子应该停在视觉地面上
    // 假设 roadSegment 纹理的高度就是地面厚度，或者直接使用 groundY
    dashParticleProps.targetGroundY = groundY + GROUND_VISUAL_OFFSET; // 粒子停在恐龙脚下的地面

    dashTrailParticles.SetGravity({0, gravity}); // 粒子系统使用恐龙的重力作为基础
    // 然后每个粒子的 gravityEffect 再去调整
}

Dinosaur::~Dinosaur() = default;

void Dinosaur::MarkAsDead()
{
    isDead = true;
    isDashing = false;
    velocity.x = 0;
}

void Dinosaur::RequestDash()
{
    if (isDashing || dashCooldownTimer > 0.0f)
    {
        return;
    }

    isDashing = true;
    dashTimer = 0.0f;
    dashCooldownTimer = dashCooldown;

    dashDirection.x = facingRight ? 1.0f : -1.0f;
    dashDirection.y = 0.0f;

    if (dashSoundHandle.frameCount > 0 && IsAudioDeviceReady())
    {
        PlaySound(dashSoundHandle);
    }
}

void Dinosaur::Update(const float deltaTime, const float worldScrollSpeed)
{
    if (isDead)
    {
        // 死亡状态下的更新
        if (!IsOnGround())
        {
            velocity.y += gravity * deltaTime;
        }
        position.y += velocity.y * deltaTime;

        // 确保死亡的恐龙停在地面上
        if (IsOnGround())
        {
            position.y = (groundY - GetHeight()) + GROUND_VISUAL_OFFSET;
            velocity.y = 0;
        }
        UpdateCollisionRect(); // 仍然需要更新碰撞盒，即使只是为了绘制
        dashTrailParticles.Update(deltaTime);
        return;
    }

    if (dashCooldownTimer > 0.0f)
    {
        dashCooldownTimer -= deltaTime;
    }

    if (isDashing)
    {
        dashTimer += deltaTime;
        if (dashTimer >= dashDuration)
        {
            isDashing = false;
        }
        else
        {
            position.x += dashDirection.x * dashSpeedMagnitude * deltaTime;

            // ---- 修改: 发射冲刺粒子 ----
            int particlesToEmit = GetRandomValue(2, 4); // 每次冲刺帧多发几个粒子
            for (int i = 0; i < particlesToEmit; ++i)
            {
                // 从恐龙身体的随机位置发射
                float dinoWidth = GetWidth();
                float dinoHeight = GetHeight();
                Vector2 particleEmitPos = {
                    position.x + GetRandomFloat(dinoWidth * 0.1f, dinoWidth * 0.9f), // X在身体范围内随机
                    position.y + GetRandomFloat(dinoHeight * 0.1f, dinoHeight * 0.9f) // Y在身体范围内随机
                };

                // 根据冲刺方向调整粒子初始喷射角度 (轻微向后上方)
                if (dashDirection.x > 0)
                {
                    // 向右冲
                    dashParticleProps.emissionAngleMin = 100.0f; // 左后偏上
                    dashParticleProps.emissionAngleMax = 170.0f;
                }
                else
                {
                    // 向左冲
                    dashParticleProps.emissionAngleMin = 10.0f; // 右后偏上
                    dashParticleProps.emissionAngleMax = 80.0f;
                }
                // --- 传递 worldScrollSpeed ---
                dashTrailParticles.Emit(particleEmitPos, 1, dashParticleProps, worldScrollSpeed);
            }
        }
    }

    // 普通移动和跳跃逻辑 (如果不在冲刺)
    if (!isDashing)
    {
        if (coyoteTimeCounter > 0.0f) coyoteTimeCounter -= deltaTime;
        if (jumpBufferCounter > 0.0f) jumpBufferCounter -= deltaTime;
        if (jumpBufferCounter <= 0.0f) jumpQueued = false;

        if (const bool onGroundBeforeVerticalMove = IsOnGround(); jumpQueued && (onGroundBeforeVerticalMove ||
            coyoteTimeCounter > 0.0f))
        {
            ExecuteJump();
        }

        float currentEffectiveGravity = gravity;
        if (isSneaking && !IsOnGround())
        {
            // 检查实际是否在空中
            currentEffectiveGravity *= sneakGravityMultiplier;
        }

        if (!IsOnGround())
        {
            // 再次检查，因为跳跃可能改变了状态
            velocity.y += currentEffectiveGravity * deltaTime;
        }
    }
    // 垂直移动始终应用
    position.y += velocity.y * deltaTime;

    if (IsOnGround())
    {
        if (velocity.y >= 0)
        {
            velocity.y = 0;
            position.y = (groundY - GetHeight()) + GROUND_VISUAL_OFFSET;

            if (isJumping)
            {
                // 刚刚从跳跃/下落状态落地
                isJumping = false;
                if (jumpQueued && jumpBufferCounter > 0.0f && !isDashing)
                {
                    ExecuteJump();
                }
            }
            if (!isDashing)
            {
                coyoteTimeCounter = coyoteTimeDuration;
            }
        }
    }
    else
    {
        // 如果不在地面上
        isJumping = true; // 确保标记为跳跃/下落状态
    }


    // 动画更新
    if (const std::vector<Texture2D>* currentFrames = GetCurrentAnimationFramesPointer(); currentFrames && !
        currentFrames->empty())
    {
        if (isJumping && !isDashing)
        {
            currentAnimFrameIndex = 0;
        }
        else if (isDashing)
        {
            currentAnimFrameIndex = 0; // 冲刺时显示跑动/潜行第一帧
        }
        else
        {
            // 在地面且不冲刺时正常播放动画
            frameTimeCounter += deltaTime;
            if (frameTimeCounter >= animationSpeed)
            {
                frameTimeCounter = 0.0f;
                currentAnimFrameIndex++;
                if (currentAnimFrameIndex >= currentFrames->size())
                {
                    currentAnimFrameIndex = 0;
                }
            }
        }
    }
    else
    {
        currentAnimFrameIndex = 0;
    }

    UpdateCollisionRect();
    dashTrailParticles.Update(deltaTime);
}

void Dinosaur::Draw() const
{
    dashTrailParticles.Draw();
    const Texture2D texToDraw = GetCurrentTextureToDraw();
    Rectangle sourceRec = {0.0f, 0.0f, static_cast<float>(texToDraw.width), static_cast<float>(texToDraw.height)};
    if (!facingRight) sourceRec.width *= -1;
    const Rectangle destRec = {
        position.x, position.y, static_cast<float>(std::abs(texToDraw.width)), static_cast<float>(texToDraw.height)
    };
    constexpr Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(texToDraw, sourceRec, destRec, origin, 0.0f, WHITE);
}

void Dinosaur::Move(const float direction, const float deltaTime)
{
    if (isDashing || isDead) return;

    float currentMoveSpeed = moveSpeed;
    if (isSneaking) { currentMoveSpeed *= 0.5f; }

    position.x += direction * currentMoveSpeed * deltaTime;

    if (direction > 0.01f) facingRight = true;
    else if (direction < -0.01f) facingRight = false;
}


bool Dinosaur::IsOnGround() const
{
    return (position.y + GetHeight() >= groundY - GROUND_VISUAL_OFFSET);
}

void Dinosaur::RequestJump()
{
    if (isDashing) return;
    jumpBufferCounter = jumpBufferDuration;
    jumpQueued = true;
}

void Dinosaur::ExecuteJump()
{
    if (jumpSoundHandle.frameCount > 0 && IsAudioDeviceReady())
    {
        PlaySound(jumpSoundHandle);
    }
    velocity.y = jumpSpeed;
    isJumping = true; // 标记为正在跳跃/下落
    coyoteTimeCounter = 0.0f; // 消耗土狼时间
    jumpQueued = false; // 消耗跳跃请求
    jumpBufferCounter = 0.0f; // 消耗跳跃缓冲
    currentAnimFrameIndex = 0; // 重置动画帧
}

void Dinosaur::StartSneaking()
{
    if (!isSneaking)
    {
        if (sneakFrames.empty() || sneakFrames[0].id == 0) return;

        const bool wasOnGround = IsOnGround();
        const float heightBeforeSneak = GetHeight();
        isSneaking = true;
        if (const float heightAfterSneak = GetHeight(); wasOnGround && heightBeforeSneak != heightAfterSneak)
        {
            position.y += (heightBeforeSneak - heightAfterSneak);
        }
        currentAnimFrameIndex = 0;
        frameTimeCounter = 0.0f;
    }
}

void Dinosaur::StopSneaking()
{
    if (isSneaking)
    {
        const bool wasOnGround = IsOnGround();
        const float heightBeforeStand = GetHeight();
        isSneaking = false;
        if (const float heightAfterStand = GetHeight(); wasOnGround && heightBeforeStand != heightAfterStand)
        {
            position.y -= (heightAfterStand - heightBeforeStand);
            if (position.y + heightAfterStand > groundY + 0.1f)
            {
                position.y = groundY - heightAfterStand;
            }
        }
        currentAnimFrameIndex = 0;
        frameTimeCounter = 0.0f;
    }
}

const std::vector<Texture2D>* Dinosaur::GetCurrentAnimationFramesPointer() const
{
    if (isSneaking && !sneakFrames.empty() && sneakFrames[0].id > 0)
    {
        return &sneakFrames;
    }
    return &runFrames;
}

Texture2D Dinosaur::GetCurrentTextureToDraw() const
{
    if (isDead && deadTexture.id > 0)
    {
        return deadTexture;
    }

    if (const std::vector<Texture2D>* frames_ptr = GetCurrentAnimationFramesPointer(); frames_ptr && !frames_ptr->
        empty())
    {
        const std::vector<Texture2D>& frames = *frames_ptr;
        const int frameIdxToUse = currentAnimFrameIndex;

        // 跳跃或冲刺时，通常显示当前状态（跑/蹲）的第一帧
        if ((isJumping || isDashing) && !isSneaking)
        {
            // 如果是跑动状态下的跳跃或冲刺
            if (!runFrames.empty() && runFrames[0].id > 0)
            {
                return runFrames[0];
            }
        }
        else if ((isJumping || isDashing) && isSneaking)
        {
            // 如果是下蹲状态下的跳跃或冲刺（如果允许）
            if (!sneakFrames.empty() && sneakFrames[0].id > 0)
            {
                return sneakFrames[0];
            }
        }
        // 正常动画播放
        if (frameIdxToUse >= 0 && frameIdxToUse < frames.size() && frames[frameIdxToUse].id > 0)
        {
            return frames[frameIdxToUse];
        }
        // Fallback到当前动画组的第一帧
        if (!frames.empty() && frames[0].id > 0)
        {
            TraceLog(LOG_WARNING, "GetCurrentTextureToDraw: Invalid frame index %d, returning frame 0.",
                     currentAnimFrameIndex);
            return frames[0];
        }
    }

    // 最终Fallback到跑动第一帧
    if (!runFrames.empty() && runFrames[0].id > 0)
    {
        TraceLog(LOG_WARNING, "GetCurrentTextureToDraw: Falling back to runFrames[0].");
        return runFrames[0];
    }
    TraceLog(LOG_ERROR, "GetCurrentTextureToDraw: No valid texture found!");
    return Texture2D{}; // 返回一个无效纹理
}

float Dinosaur::GetHeight() const
{
    if (isDead && deadTexture.id > 0)
    {
        return static_cast<float>(deadTexture.height);
    }
    if (isSneaking && sneakHeight > 0)
    {
        return sneakHeight;
    }
    return runHeight;
}

float Dinosaur::GetWidth() const
{
    if (isDead && deadTexture.id > 0)
    {
        return static_cast<float>(deadTexture.width);
    }
    if (const Texture2D tex = GetCurrentTextureToDraw(); tex.id > 0) return static_cast<float>(std::abs(tex.width));
    if (!runFrames.empty() && runFrames[0].id > 0) return static_cast<float>(runFrames[0].width);
    return 44.0f;
}

void Dinosaur::UpdateCollisionRect()
{
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    collisionRect.width = GetWidth();
    collisionRect.height = GetHeight();
}

Rectangle Dinosaur::GetCollisionRect() const
{
    float widthReductionFactor = 0.40f;
    float heightReductionFactorTop = 0.25f;
    float heightReductionFactorBottom = 0.15f;

    if (isSneaking)
    {
        widthReductionFactor = 0.45f;
        heightReductionFactorTop = 0.30f;
        heightReductionFactorBottom = 0.10f;
    }
    else if (isDashing)
    {
        widthReductionFactor = 0.50f;
        heightReductionFactorTop = 0.35f;
        heightReductionFactorBottom = 0.10f;
    }

    const float horizontalPadding = collisionRect.width * widthReductionFactor;
    const float verticalPaddingTop = collisionRect.height * heightReductionFactorTop;
    const float verticalPaddingBottom = collisionRect.height * heightReductionFactorBottom;

    Rectangle adjustedRect = {
        collisionRect.x + horizontalPadding / 2.0f,
        collisionRect.y + verticalPaddingTop,
        collisionRect.width - horizontalPadding,
        collisionRect.height - (verticalPaddingTop + verticalPaddingBottom)
    };

    if (adjustedRect.width < 1.0f) adjustedRect.width = 1.0f;
    if (adjustedRect.height < 1.0f) adjustedRect.height = 1.0f;

    return adjustedRect;
}
