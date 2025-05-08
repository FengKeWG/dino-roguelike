#include "../include/Dinosaur.h"
#include <iostream>
#include <cmath>
#include "raylib.h" // 确保包含

Dinosaur::Dinosaur(const float startX, const float initialGroundY,
                   const std::vector<Texture2D>& runTex,
                   const std::vector<Texture2D>& sneakTex,
                   const Sound& jumpSound)
    : position({0, 0}), velocity({0, 0}), gravity(1800.0f), jumpSpeed(-600.0f),
      moveSpeed(280.0f), jumpSoundHandle(jumpSound),
      isJumping(false),
      isSneaking(false), facingRight(true),
      collisionRect({0, 0, 0, 0}), runFrames(runTex), sneakFrames(sneakTex),
      currentAnimFrameIndex(0),
      frameTimeCounter(0.0f), animationSpeed(0.08f),
      groundY(initialGroundY), runHeight(0.0f), sneakHeight(0.0f),
      coyoteTimeCounter(0.0f), jumpBufferCounter(0.0f),
      jumpQueued(false),
      sneakGravityMultiplier(2.5f)
{
    // 计算高度
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

Dinosaur::~Dinosaur() = default;

bool Dinosaur::IsOnGround() const
{
    return (position.y + GetHeight() >= groundY - 0.5f);
}

void Dinosaur::RequestJump()
{
    jumpBufferCounter = jumpBufferDuration;
    jumpQueued = true;
}

void Dinosaur::ExecuteJump()
{
    // 播放声音现在在这里
    if (jumpSoundHandle.frameCount > 0 && IsAudioDeviceReady())
    {
        PlaySound(jumpSoundHandle);
    }

    velocity.y = jumpSpeed;
    isJumping = true;
    coyoteTimeCounter = 0.0f;
    jumpQueued = false;
    jumpBufferCounter = 0.0f;
    currentAnimFrameIndex = 0; // 动画重置到跑动/下蹲第一帧
}

void Dinosaur::Update(const float deltaTime)
{
    if (coyoteTimeCounter > 0.0f) coyoteTimeCounter -= deltaTime;
    if (jumpBufferCounter > 0.0f) jumpBufferCounter -= deltaTime;
    if (jumpBufferCounter <= 0.0f) jumpQueued = false;

    bool onGround = IsOnGround();

    if (jumpQueued && (onGround || coyoteTimeCounter > 0.0f))
    {
        ExecuteJump();
        onGround = false;
        isJumping = true;
    }

    float currentEffectiveGravity = gravity;
    if (isSneaking && !onGround) // 如果在下蹲状态并且在空中
    {
        currentEffectiveGravity *= sneakGravityMultiplier; // 应用更大的重力
    }

    if (!onGround)
    {
        velocity.y += currentEffectiveGravity * deltaTime; // <<< 修改：使用 currentEffectiveGravity
    }
    position.y += velocity.y * deltaTime;
    onGround = IsOnGround();

    if (onGround)
    {
        if (velocity.y >= 0)
        {
            // 确认是落地或静止在地面
            velocity.y = 0;
            // *** 修改这里：让恐龙稍微“嵌入”地面 ***
            float visualOffset = 5.0f; // 向下嵌入1个像素 (可调整)
            position.y = (groundY - GetHeight()) + visualOffset; // Y坐标向下微调
            // 确保不会因为调整而导致 IsOnGround() 判定错误（理论上不会，因为 IsOnGround 容差更大）
            if (isJumping)
            {
                // 如果是从跳跃/下落状态刚刚落地
                isJumping = false;
                // 检查缓冲跳跃
                if (jumpQueued && jumpBufferCounter > 0.0f)
                {
                    ExecuteJump();
                }
            }
            coyoteTimeCounter = coyoteTimeDuration; // 刷新土狼时间
        }
    }
    else
    {
        // 如果计算后仍在空中
        isJumping = true; // 确保标记为跳跃/下落状态
    }

    // --- 动画更新 ---
    if (const std::vector<Texture2D>* currentFrames = GetCurrentAnimationFramesPointer(); currentFrames && !
        currentFrames->empty())
    {
        // 无论是否跳跃，只要不在地面，就固定动画帧（或者你可以设计跳跃动画）
        // 当前设计是空中时显示第一帧
        if (isJumping)
        {
            currentAnimFrameIndex = 0;
        }
        else
        {
            // 在地面时正常播放动画
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
}

void Dinosaur::StartSneaking()
{
    if (!isSneaking)
    {
        // 只有在非下蹲状态才执行切换
        if (sneakFrames.empty() || sneakFrames[0].id == 0) return; // 无效下蹲帧则不切换

        const bool wasOnGround = IsOnGround(); // 记录切换前的地面状态
        const float heightBeforeSneak = GetHeight(); // 获取切换前的高度 (runHeight)

        isSneaking = true; // *** 切换标志 ***

        // *** 关键：如果在地面上，手动调整Y坐标以保持脚部位置 ***
        if (const float heightAfterSneak = GetHeight(); wasOnGround && heightBeforeSneak != heightAfterSneak)
        {
            position.y += (heightBeforeSneak - heightAfterSneak); // 向下移动，补偿高度减少
        }
        // 如果在空中切换，不调整Y坐标，只改变碰撞盒和视觉

        currentAnimFrameIndex = 0; // 重置下蹲动画
        frameTimeCounter = 0.0f;
        // UpdateCollisionRect(); // Update() 末尾会调用
    }
}

void Dinosaur::StopSneaking()
{
    if (isSneaking)
    {
        // 只有在下蹲状态才执行切换
        // if (sneakFrames.empty() || sneakFrames[0].id == 0) { isSneaking = false; return; } // 不需要检查，因为isSneaking为true

        const bool wasOnGround = IsOnGround(); // 记录切换前的地面状态
        const float heightBeforeStand = GetHeight(); // 获取切换前的高度 (sneakHeight)

        isSneaking = false; // *** 切换标志 ***

        // *** 关键：如果在地面上，手动调整Y坐标以保持脚部位置 ***
        if (const float heightAfterStand = GetHeight(); wasOnGround && heightBeforeStand != heightAfterStand)
        {
            position.y -= (heightAfterStand - heightBeforeStand); // 向上移动，补偿高度增加
            // 微调以防因浮点数误差陷入地下，虽然Update中的吸附应该能处理
            if (position.y + heightAfterStand > groundY + 0.1f)
            {
                // 加一点容差
                position.y = groundY - heightAfterStand;
            }
        }
        // 如果在空中恢复站立，Y坐标不手动调整

        currentAnimFrameIndex = 0; // 重置跑动动画
        frameTimeCounter = 0.0f;
        // UpdateCollisionRect(); // Update() 末尾会调用
    }
}

const std::vector<Texture2D>* Dinosaur::GetCurrentAnimationFramesPointer() const
{
    // 优先返回下蹲帧（如果正在下蹲且有效）
    if (isSneaking && !sneakFrames.empty() && sneakFrames[0].id > 0)
    {
        return &sneakFrames;
    }
    // 否则返回跑动帧
    return &runFrames;
}

Texture2D Dinosaur::GetCurrentTextureToDraw() const
{
    if (const std::vector<Texture2D>* frames_ptr = GetCurrentAnimationFramesPointer(); frames_ptr && !frames_ptr->
        empty())
    {
        const std::vector<Texture2D>& frames = *frames_ptr;
        int frameIdxToUse = currentAnimFrameIndex;

        // 如果在空中并且 *不是* 下蹲状态，强制使用跑动第一帧作为跳跃帧
        if (isJumping && !isSneaking)
        {
            if (!runFrames.empty() && runFrames[0].id > 0)
            {
                // 确保跑动帧有效
                frameIdxToUse = 0;
                // 直接返回跑动第一帧
                return runFrames[0];
            }
            TraceLog(LOG_ERROR, "Cannot get jump texture, runFrames missing!");
            return Texture2D{};
        }

        // 对于跑动或下蹲状态（地面或空中）
        if (frameIdxToUse >= 0 && frameIdxToUse < frames.size() && frames[frameIdxToUse].id > 0)
        {
            return frames[frameIdxToUse];
        }
        if (!frames.empty() && frames[0].id > 0)
        {
            // 索引无效时回退到当前动画组的第一帧
            TraceLog(LOG_WARNING,
                     "GetCurrentTextureToDraw: Invalid frame index %d for current state, returning frame 0.",
                     currentAnimFrameIndex);
            return frames[0];
        }
    }

    // 最终 Fallback
    if (!runFrames.empty() && runFrames[0].id > 0)
    {
        TraceLog(LOG_WARNING, "GetCurrentTextureToDraw: Falling back to runFrames[0].");
        return runFrames[0];
    }
    TraceLog(LOG_ERROR, "GetCurrentTextureToDraw: No valid texture found!");
    return Texture2D{0};
}

float Dinosaur::GetHeight() const
{
    if (isSneaking && sneakHeight > 0)
    {
        return sneakHeight;
    }
    return runHeight;
}

// GetWidth, Move, Draw, UpdateCollisionRect, GetCollisionRect 保持不变
// ... (粘贴上一版本中这些方法的实现)
float Dinosaur::GetWidth() const
{
    if (const Texture2D tex = GetCurrentTextureToDraw(); tex.id > 0) return static_cast<float>(std::abs(tex.width));
    if (!runFrames.empty() && runFrames[0].id > 0) return static_cast<float>(runFrames[0].width);
    return 44.0f;
}

void Dinosaur::Move(const float direction, const float deltaTime)
{
    float currentMoveSpeed = moveSpeed;
    if (isSneaking) { currentMoveSpeed *= 0.5f; }
    position.x += direction * currentMoveSpeed * deltaTime;
    if (direction > 0) facingRight = true;
    else if (direction < 0) facingRight = false;
}

void Dinosaur::Draw() const
{
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
    const Rectangle originalRect = collisionRect;
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
