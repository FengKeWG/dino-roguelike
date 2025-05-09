#include "../include/Dinosaur.h" // 确保路径正确
#include <iostream>
#include <cmath>
#include "raylib.h"
#include "raymath.h"

const float GROUND_VISUAL_OFFSET = 5.0f;

Dinosaur::Dinosaur(const float startX, const float initialGroundY,
                   const std::vector<Texture2D>& runTex,
                   const std::vector<Texture2D>& sneakTex,
                   const Sound& jumpSound,
                   const Sound& dashSound)
    : position({0, 0}), velocity({0, 0}), gravity(1800.0f), jumpSpeed(-600.0f),
      moveSpeed(280.0f), jumpSoundHandle(jumpSound), dashSoundHandle(dashSound),
      isJumping(false),
      isSneaking(false), facingRight(true),
      collisionRect({0, 0, 0, 0}), runFrames(runTex), sneakFrames(sneakTex),
      currentAnimFrameIndex(0),
      frameTimeCounter(0.0f), animationSpeed(0.08f),
      groundY(initialGroundY), runHeight(0.0f), sneakHeight(0.0f),
      coyoteTimeCounter(0.0f), jumpBufferCounter(0.0f),
      jumpQueued(false),
      sneakGravityMultiplier(2.5f),
      // 初始化冲刺属性
      isDashing(false),
      dashSpeedMagnitude(800.0f),
      dashDuration(0.18f),
      dashTimer(0.0f),
      dashCooldown(0.5f),
      dashCooldownTimer(0.0f),
      dashDirection({0.0f, 0.0f}),
      particleBaseGravity(1200.0f)
{
    if (!runFrames.empty() && runFrames[0].id > 0) { runHeight = static_cast<float>(runFrames[0].height); }
    else
    {
        TraceLog(LOG_ERROR, "CRITICAL: Dinosaur needs run frames for height!");
        runHeight = 50.0f;
    }
    if (!sneakFrames.empty() && sneakFrames[0].id > 0) { sneakHeight = static_cast<float>(sneakFrames[0].height); }
    else
    {
        sneakHeight = runHeight * 0.6f;
        TraceLog(LOG_WARNING, "Sneak frames missing or invalid, estimating height.");
    }

    position = {startX, initialGroundY - runHeight};
    UpdateCollisionRect();
}

Dinosaur::~Dinosaur() = default; // 没有需要特殊清理的资源（粒子是值类型）

void Dinosaur::RequestDash()
{
    if (isDashing || dashCooldownTimer > 0.0f)
    {
        return; // 正在冲刺或冷却中
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

void Dinosaur::Update(float deltaTime, float worldScrollSpeed)
{
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
            // 在发射粒子时传递世界滚动速度，以便粒子落地后使用
            EmitDashParticleTrail(worldScrollSpeed);
        }
    }

    // 普通移动和跳跃逻辑 (如果不在冲刺)
    if (!isDashing)
    {
        if (coyoteTimeCounter > 0.0f) coyoteTimeCounter -= deltaTime;
        if (jumpBufferCounter > 0.0f) jumpBufferCounter -= deltaTime;
        if (jumpBufferCounter <= 0.0f) jumpQueued = false;

        bool onGroundBeforeVerticalMove = IsOnGround();

        if (jumpQueued && (onGroundBeforeVerticalMove || coyoteTimeCounter > 0.0f))
        {
            ExecuteJump();
            // onGroundBeforeVerticalMove = false; // ExecuteJump 会设置 isJumping
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


    // 地面检测和处理
    bool onGroundAfterUpdate = IsOnGround();
    if (onGroundAfterUpdate)
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
    UpdateParticles(deltaTime, worldScrollSpeed);
}

void Dinosaur::EmitDashParticleTrail(float currentWorldScrollSpeed)
{
    int particlesToEmit = GetRandomValue(2, 3);
    for (int i = 0; i < particlesToEmit; ++i)
    {
        DashParticle p;
        float dinoWidth = GetWidth();
        float dinoHeight = GetHeight();

        // ... (粒子初始位置与之前相同)
        float randomXOffsetFactor = (GetRandomValue(0, 100) / 100.0f);
        if (dashDirection.x > 0)
        {
            p.position.x = position.x + dinoWidth * (randomXOffsetFactor * 0.8f);
        }
        else
        {
            p.position.x = position.x + dinoWidth * (0.2f + randomXOffsetFactor * 0.8f);
        }
        float randomYOffsetFactor = (GetRandomValue(0, 100) / 100.0f);
        p.position.y = position.y + dinoHeight * randomYOffsetFactor;


        // ... (粒子初始速度与之前相同)
        float baseSpeedX = (float)GetRandomValue(50, 120);
        float baseSpeedY = (float)GetRandomValue(80, 180);
        float angleVariation = (GetRandomValue(-20, 20) * DEG2RAD);
        p.velocity.x = -dashDirection.x * baseSpeedX * cos(angleVariation) - dashDirection.y * baseSpeedY * sin(
            angleVariation);
        p.velocity.y = -dashDirection.x * baseSpeedX * sin(angleVariation) + dashDirection.y * baseSpeedY *
            cos(angleVariation) - baseSpeedY;


        // --- 关键：调整生命周期，确保足够长以便大部分能落地 ---
        p.initialLife = (float)GetRandomValue(80, 150) / 100.0f; // 0.8 到 1.5 秒
        // 这个值需要足够大，让粒子有时间在各种初始速度和高度下到达地面
        p.lifeRemaining = p.initialLife;
        p.size = (float)GetRandomValue(2, 4);

        const unsigned char maxBaseColor = 83;
        unsigned char baseShade = GetRandomValue(maxBaseColor - 25, maxBaseColor);
        if (baseShade < 25) baseShade = 25;
        p.color = {
            (unsigned char)(baseShade + GetRandomValue(-8, 4)),
            (unsigned char)(baseShade + GetRandomValue(-8, 4)),
            (unsigned char)(baseShade + GetRandomValue(-8, 4)),
            255 // --- 关键：初始Alpha设为完全不透明 ---
        };
        p.color.r = Clamp(p.color.r, 0, 255);
        p.color.g = Clamp(p.color.g, 0, 255);
        p.color.b = Clamp(p.color.b, 0, 255);

        p.rotation = (float)GetRandomValue(0, 359);
        p.angularVelocity = (float)GetRandomValue(-250, 250);
        p.gravityEffect = (float)GetRandomValue(90, 110) / 100.0f;

        p.isLanded = false;
        p.landedScrollSpeedX = -currentWorldScrollSpeed;

        particles.push_back(p);
    }
}

void Dinosaur::UpdateParticles(float deltaTime, float worldScrollSpeed)
{
    float particleGroundY = groundY + GROUND_VISUAL_OFFSET;

    for (auto it = particles.begin(); it != particles.end(); /* no increment */)
    {
        // 检查是否移出屏幕左侧
        if (it->position.x + it->size < -50)
        {
            it = particles.erase(it);
            continue;
        }

        if (!it->isLanded)
        {
            // --- 飞行中的粒子 ---
            it->lifeRemaining -= deltaTime;
            if (it->lifeRemaining <= 0.0f)
            {
                // 如果飞行生命周期耗尽，则移除
                it = particles.erase(it);
                continue;
            }

            // 更新物理状态
            it->velocity.y += particleBaseGravity * it->gravityEffect * deltaTime;
            it->position = Vector2Add(it->position, Vector2Scale(it->velocity, deltaTime));
            it->rotation += it->angularVelocity * deltaTime;

            // --- 关键：飞行时不改变颜色或Alpha ---
            // (确保这里没有修改 it->color.r, g, b, 或 a 的代码)
            // 它们将保持 EmitDashParticleTrail 中设置的初始颜色和Alpha (255)

            // 检测落地
            if (it->position.y + it->size >= particleGroundY && it->velocity.y > 0)
            {
                it->isLanded = true;
                it->position.y = particleGroundY - it->size;
                it->velocity.y = 0;
                it->velocity.x = it->landedScrollSpeedX;
                it->angularVelocity *= 0.3f;
                // 落地后，lifeRemaining 的作用改变，主要由出屏决定其移除
                // 但我们可以给它一个非常大的值，确保它不会因为这个先消失
                it->lifeRemaining = 999.0f; // 一个象征性的超长值
                // 落地后颜色和Alpha保持不变 (即初始值)
            }
        }
        else
        {
            // --- 已落地的粒子 ---
            it->position.x += it->landedScrollSpeedX * deltaTime;
            it->rotation += it->angularVelocity * deltaTime * 0.1f;
            if (std::abs(it->angularVelocity) > 0.1f)
            {
                it->angularVelocity *= 0.98f;
            }
            else
            {
                it->angularVelocity = 0;
            }
            // 颜色和Alpha保持不变
        }
        ++it;
    }
}


void Dinosaur::DrawParticles() const
{
    for (const auto& p : particles)
    {
        // 使用 DrawRectanglePro 来支持旋转
        DrawRectanglePro(
            {p.position.x, p.position.y, p.size, p.size}, // 目标矩形
            {p.size / 2, p.size / 2}, // 旋转中心点 (方块中心)
            p.rotation, // 旋转角度
            p.color // 颜色
        );
    }
}

void Dinosaur::Draw() const
{
    DrawParticles(); // 绘制粒子（在恐龙之前，所以粒子在恐龙下方/之后）

    if (const Texture2D texToDraw = GetCurrentTextureToDraw(); texToDraw.id > 0)
    {
        Rectangle sourceRec = {0.0f, 0.0f, static_cast<float>(texToDraw.width), static_cast<float>(texToDraw.height)};
        if (!facingRight) sourceRec.width *= -1;
        const Rectangle destRec = {
            position.x, position.y, static_cast<float>(std::abs(texToDraw.width)), static_cast<float>(texToDraw.height)
        };
        constexpr Vector2 origin = {0.0f, 0.0f};
        DrawTexturePro(texToDraw, sourceRec, destRec, origin, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleRec(GetCollisionRect(), LIME);
    }
    // 调试用：
    // DrawRectangleLinesEx(GetCollisionRect(), 1, BLUE);
}

void Dinosaur::Move(const float direction, const float deltaTime)
{
    if (isDashing) return;

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
    if (isDashing) return; // 冲刺时不允许跳跃 (可选行为)
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
        const float heightAfterSneak = GetHeight();
        if (wasOnGround && heightBeforeSneak != heightAfterSneak)
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
        const float heightAfterStand = GetHeight();
        if (wasOnGround && heightBeforeStand != heightAfterStand)
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
    const std::vector<Texture2D>* frames_ptr = GetCurrentAnimationFramesPointer();

    if (frames_ptr && !frames_ptr->empty())
    {
        const std::vector<Texture2D>& frames = *frames_ptr;
        int frameIdxToUse = currentAnimFrameIndex;

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
    return Texture2D{0}; // 返回一个无效纹理
}

float Dinosaur::GetHeight() const
{
    if (isSneaking && sneakHeight > 0)
    {
        return sneakHeight;
    }
    return runHeight;
}

float Dinosaur::GetWidth() const
{
    Texture2D tex = GetCurrentTextureToDraw();
    if (tex.id > 0) return static_cast<float>(std::abs(tex.width));

    // Fallback if GetCurrentTextureToDraw returns invalid texture
    if (!runFrames.empty() && runFrames[0].id > 0) return static_cast<float>(runFrames[0].width);
    return 44.0f; // Default width
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
    Rectangle originalRect = collisionRect; // 使用成员变量，它在UpdateCollisionRect中更新
    Rectangle adjustedRect = originalRect;

    float widthReductionFactor = 0.25f;
    float heightReductionFactorTop = 0.15f;
    float heightReductionFactorBottom = 0.08f;

    if (isSneaking)
    {
        widthReductionFactor = 0.30f;
        heightReductionFactorTop = 0.20f;
        heightReductionFactorBottom = 0.05f;
    }
    if (isDashing)
    {
        // 冲刺时碰撞盒可以更小，更贴近地面
        widthReductionFactor = 0.35f;
        heightReductionFactorTop = 0.25f; // 顶部压得更低
        heightReductionFactorBottom = 0.05f;
    }

    const float horizontalPadding = originalRect.width * widthReductionFactor;
    const float verticalPaddingTop = originalRect.height * heightReductionFactorTop;
    const float verticalPaddingBottom = originalRect.height * heightReductionFactorBottom;

    adjustedRect.x += horizontalPadding / 2.0f;
    adjustedRect.width -= horizontalPadding;
    adjustedRect.y += verticalPaddingTop;
    adjustedRect.height -= (verticalPaddingTop + verticalPaddingBottom);

    if (adjustedRect.width < 1.0f) adjustedRect.width = 1.0f;
    if (adjustedRect.height < 1.0f) adjustedRect.height = 1.0f;

    return adjustedRect;
}
