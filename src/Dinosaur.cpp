// src/Dinosaur.cpp
#include "../include/Dinosaur.h"

Dinosaur::Dinosaur(const float startX, const float groundY,
                   const std::vector<Texture2D>& runTex,
                   const std::vector<Texture2D>& sneakTex,
                   const Texture2D& deadTex,
                   const Sound& jumpSound,
                   const Sound& dashSound)
    : position({0, 0}), velocity({0, 0}), groundY(groundY), runHeight(0.0f),
      sneakHeight(0.0f), jumpSoundHandle(jumpSound), dashSoundHandle(dashSound),
      isJumping(false), isSneaking(false), facingRight(true),
      runFrames(runTex), sneakFrames(sneakTex), deadTexture(deadTex),
      isDead(false), currentAnimFrameIndex(0), frameTimeCounter(0.0f),
      animationSpeed(0.08f), collisionRect({0, 0, 0, 0}),
      gravity(1800.0f), jumpSpeed(-600.0f), coyoteTimeDuration(0.1f),
      coyoteTimeCounter(0.0f), jumpBufferDuration(0.1f),
      jumpBufferCounter(0.0f),
      jumpQueued(false),
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
    position = {startX, groundY - runHeight};
    // 更新碰撞矩形
    UpdateCollisionRect();

    // 配置冲刺拖尾粒子的属性
    dashParticleProps.lifeTimeMin = 2.0f;
    dashParticleProps.lifeTimeMax = 5.0f;
    dashParticleProps.initialSpeedMin = 30.0f;
    dashParticleProps.initialSpeedMax = 100.0f;
    dashParticleProps.startSizeMin = 2.0f;
    dashParticleProps.startSizeMax = 4.0f;
    dashParticleProps.startColor = {80, 80, 80, 200};
    dashParticleProps.angularVelocityMin = -220.0f;
    dashParticleProps.angularVelocityMax = 220.0f;
    dashParticleProps.gravityScaleMin = 0.1f;
    dashParticleProps.gravityScaleMax = 0.5f;
    dashParticleProps.targetGroundY = groundY + 5.0f;
    dashTrailParticles.SetGravity({0, gravity});
}

Dinosaur::~Dinosaur() = default;

// 标记恐龙为死亡状态
void Dinosaur::MarkAsDead()
{
    isDead = true;
    isDashing = false;
    velocity.x = 0;
}

// 请求冲刺
void Dinosaur::RequestDash()
{
    // 如果正在冲刺或冲刺在冷却中，则不执行
    if (isDashing || dashCooldownTimer > 0.0f)
    {
        return;
    }
    isDashing = true;
    dashTimer = 0.0f; // 重置冲刺计时器
    dashCooldownTimer = dashCooldown; // 开始冲刺冷却
    dashDirection.x = facingRight ? 1.0f : -1.0f; // 根据朝向设置冲刺方向
    dashDirection.y = 0.0f;
    PlaySound(dashSoundHandle);
}

// 更新恐龙状态，每帧调用
void Dinosaur::Update(const float deltaTime, const float worldScrollSpeed)
{
    // 如果恐龙已死亡
    if (isDead)
    {
        // 如果不在地面上，施加重力
        if (!IsOnGround())
        {
            velocity.y += gravity * deltaTime;
        }
        position.y += velocity.y * deltaTime; // 更新Y轴位置
        // 如果接触地面
        if (IsOnGround())
        {
            position.y = (groundY - GetHeight()) + 5.0f;
            velocity.y = 0;
        }
        UpdateCollisionRect(); // 更新碰撞矩形
        dashTrailParticles.Update(deltaTime);
        return;
    }

    // 更新冲刺冷却计时器
    if (dashCooldownTimer > 0.0f)
    {
        dashCooldownTimer -= deltaTime;
    }

    // 如果正在冲刺
    if (isDashing)
    {
        dashTimer += deltaTime; // 增加冲刺时间
        if (dashTimer >= dashDuration) // 如果冲刺时间结束
        {
            isDashing = false; // 结束冲刺状态
        }
        else
        {
            // 更新X轴位置实现冲刺移动
            position.x += dashDirection.x * dashSpeedMagnitude * deltaTime;
            // 发射冲刺拖尾粒子
            const int particlesToEmit = randI(2, 4); // 随机发射
            for (int i = 0; i < particlesToEmit; ++i)
            {
                const float dinoWidth = GetWidth();
                const float dinoHeight = GetHeight();
                // 在恐龙身体范围内随机位置发射粒子
                const Vector2 particleEmitPos = {
                    position.x + randF(dinoWidth * 0.1f, dinoWidth * 0.9f),
                    position.y + randF(dinoHeight * 0.1f, dinoHeight * 0.9f)
                };
                // 根据冲刺方向设置粒子发射角度，使其向后飞散
                if (dashDirection.x > 0) // 向右冲刺
                {
                    dashParticleProps.emissionAngleMin = 100.0f;
                    dashParticleProps.emissionAngleMax = 170.0f;
                }
                else
                {
                    dashParticleProps.emissionAngleMin = 10.0f;
                    dashParticleProps.emissionAngleMax = 80.0f;
                }
                dashTrailParticles.Emit(particleEmitPos, 1, dashParticleProps, worldScrollSpeed);
            }
        }
    }

    // 如果不处于冲刺状态
    if (!isDashing)
    {
        // 更新土狼时间和跳跃缓冲计时器
        if (coyoteTimeCounter > 0.0f) coyoteTimeCounter -= deltaTime;
        if (jumpBufferCounter > 0.0f) jumpBufferCounter -= deltaTime;
        if (jumpBufferCounter <= 0.0f) jumpQueued = false; // 跳跃缓冲超时，取消已缓存的跳跃

        // 检查是否可以执行跳跃 (有跳跃请求，且在地面或土狼时间内)
        if (const bool onGroundBeforeVerticalMove = IsOnGround(); jumpQueued && (onGroundBeforeVerticalMove ||
            coyoteTimeCounter > 0.0f))
        {
            ExecuteJump(); // 执行跳跃
        }

        // 计算当前有效的重力
        float currentEffectiveGravity = gravity;
        // 如果在潜行且不在地面，增加重力使其下落更快
        if (isSneaking && !IsOnGround())
        {
            currentEffectiveGravity *= sneakGravityMultiplier;
        }
        // 如果不在地面，施加重力
        if (!IsOnGround())
        {
            velocity.y += currentEffectiveGravity * deltaTime;
        }
    }

    position.y += velocity.y * deltaTime;

    // 处理着地逻辑
    if (IsOnGround())
    {
        if (velocity.y >= 0) // 确保是向下运动时触地
        {
            velocity.y = 0; // 垂直速度清零
            position.y = (groundY - GetHeight()) + 5.0f; // 精确设置在地面上
            if (isJumping) // 如果之前在跳跃状态
            {
                isJumping = false; // 结束跳跃状态
                // 如果着地时仍有跳跃请求在缓冲期内且不在冲刺，则立即再次跳跃 (用于连续跳)
                if (jumpQueued && jumpBufferCounter > 0.0f && !isDashing)
                {
                    ExecuteJump();
                }
            }
            // 着地时重置土狼时间 (不在冲刺时)
            if (!isDashing)
            {
                coyoteTimeCounter = coyoteTimeDuration;
            }
        }
    }
    else // 如果不在地面上
    {
        isJumping = true; // 标记为跳跃状态 (或空中状态)
    }

    // 更新动画帧
    if (const std::vector<Texture2D>* currentFrames = GetCurrentAnimationFramesPointer(); currentFrames && !
        currentFrames->empty()) // 获取当前状态的动画帧组
    {
        if (isJumping && !isDashing)
        {
            currentAnimFrameIndex = 0;
        }
        else if (isDashing)
        {
            currentAnimFrameIndex = 0;
        }
        else
        {
            frameTimeCounter += deltaTime;
            if (frameTimeCounter >= animationSpeed)
            {
                frameTimeCounter = 0.0f;
                currentAnimFrameIndex++;
                if (currentAnimFrameIndex >= currentFrames->size())
                {
                    currentAnimFrameIndex = 0; // 循环播放
                }
            }
        }
    }
    else
    {
        currentAnimFrameIndex = 0;
    }

    // 更新碰撞矩形
    UpdateCollisionRect();
    // 更新冲刺粒子系统
    dashTrailParticles.Update(deltaTime);
}

// 绘制恐龙
void Dinosaur::Draw() const
{
    dashTrailParticles.Draw();
    const Texture2D texToDraw = GetCurrentTextureToDraw(); // 获取当前应绘制的纹理
    // 定义源矩形 (纹理的哪个部分被绘制)
    Rectangle sourceRec = {0.0f, 0.0f, static_cast<float>(texToDraw.width), static_cast<float>(texToDraw.height)};
    // 如果恐龙朝左，则水平翻转源矩形
    if (!facingRight) sourceRec.width *= -1;
    // 定义目标矩形 (在屏幕上的绘制位置和大小)
    const Rectangle destRec = {
        position.x, position.y, static_cast<float>(std::abs(texToDraw.width)), static_cast<float>(texToDraw.height)
    };
    constexpr Vector2 origin = {0.0f, 0.0f}; // 旋转和缩放的原点 (左上角)
    DrawTexturePro(texToDraw, sourceRec, destRec, origin, 0.0f, WHITE);
}

// 控制恐龙左右移动
void Dinosaur::Move(const float direction, const float deltaTime)
{
    if (isDashing || isDead) return;

    float currentMoveSpeed = moveSpeed;
    // 如果在潜行，移动速度减半
    if (isSneaking) { currentMoveSpeed *= 0.5f; }
    // 根据方向、速度和时间差更新X轴位置
    position.x += direction * currentMoveSpeed * deltaTime;

    // 根据移动方向更新朝向
    if (direction > 0.01f) facingRight = true;
    else if (direction < -0.01f) facingRight = false;
}

// 检查恐龙是否在地面上
bool Dinosaur::IsOnGround() const
{
    return (position.y + GetHeight() >= groundY - 5.0f);
}

// 请求跳跃
void Dinosaur::RequestJump()
{
    if (isDashing) return;
    jumpBufferCounter = jumpBufferDuration; // 开始跳跃缓冲计时
    jumpQueued = true;
}

// 执行跳跃
void Dinosaur::ExecuteJump()
{
    velocity.y = jumpSpeed; // 设置向上的初始速度
    isJumping = true;
    coyoteTimeCounter = 0.0f; // 消耗土狼时间
    jumpQueued = false; // 消耗已缓存的跳跃请求
    jumpBufferCounter = 0.0f; // 重置跳跃缓冲计时器
    currentAnimFrameIndex = 0;
    PlaySound(jumpSoundHandle);
}

// 开始潜行
void Dinosaur::StartSneaking()
{
    if (!isSneaking)
    {
        const bool wasOnGround = IsOnGround();
        const float heightBeforeSneak = GetHeight();
        isSneaking = true;
        // 如果在地面上，并且潜行前后高度发生变化，调整Y位置以保持脚底位置不变
        if (const float heightAfterSneak = GetHeight(); wasOnGround && heightBeforeSneak != heightAfterSneak)
        {
            position.y += (heightBeforeSneak - heightAfterSneak);
        }
        currentAnimFrameIndex = 0;
        frameTimeCounter = 0.0f;
    }
}

// 停止潜行
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
            // 防止站起后陷入地面
            if (position.y + heightAfterStand > groundY + 0.1f)
            {
                position.y = groundY - heightAfterStand;
            }
        }
        currentAnimFrameIndex = 0;
        frameTimeCounter = 0.0f;
    }
}

// 获取当前动画应该使用的帧序列的指针
const std::vector<Texture2D>* Dinosaur::GetCurrentAnimationFramesPointer() const
{
    if (isSneaking)
    {
        return &sneakFrames;
    }
    return &runFrames;
}

// 获取当前应该绘制的单个纹理
Texture2D Dinosaur::GetCurrentTextureToDraw() const
{
    if (isDead)
    {
        return deadTexture;
    }

    // 获取当前状态对应的动画帧组
    if (const std::vector<Texture2D>* frames_ptr = GetCurrentAnimationFramesPointer(); frames_ptr && !frames_ptr->
        empty())
    {
        const std::vector<Texture2D>& frames = *frames_ptr;
        const int frameIdxToUse = currentAnimFrameIndex;

        // 特殊处理跳跃或冲刺时的帧 (通常是固定的第一帧)
        if ((isJumping || isDashing) && !isSneaking) // 跳跃/冲刺 且 不潜行
        {
            return runFrames[0];
        }
        if ((isJumping || isDashing) && isSneaking) // 跳跃/冲刺 且 潜行
        {
            return sneakFrames[0];
        }

        // 正常播放动画帧
        if (frameIdxToUse >= 0 && frameIdxToUse < frames.size() && frames[frameIdxToUse].id > 0)
        {
            return frames[frameIdxToUse];
        }
    }
    return Texture2D{};
}

// 获取恐龙当前的高度
float Dinosaur::GetHeight() const
{
    if (isDead)
    {
        return static_cast<float>(deadTexture.height);
    }
    if (isSneaking && sneakHeight > 0)
    {
        return sneakHeight;
    }
    return runHeight;
}

// 获取恐龙当前的宽度
float Dinosaur::GetWidth() const
{
    if (isDead)
    {
        return static_cast<float>(deadTexture.width);
    }
    if (const Texture2D tex = GetCurrentTextureToDraw(); tex.id > 0) return static_cast<float>(std::abs(tex.width));
    return static_cast<float>(runFrames[0].width);
}

// 更新碰撞矩形的位置和大小
void Dinosaur::UpdateCollisionRect()
{
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    collisionRect.width = GetWidth();
    collisionRect.height = GetHeight();
}

// 获取用于碰撞检测的、调整过的矩形
Rectangle Dinosaur::GetCollisionRect() const
{
    float widthReductionFactor = 0.40f; // 水平方向缩减40%
    float heightReductionFactorTop = 0.25f; // 顶部缩减25%
    float heightReductionFactorBottom = 0.15f; // 底部缩减15%

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

    // 创建调整后的碰撞矩形
    Rectangle adjustedRect = {
        collisionRect.x + horizontalPadding / 2.0f, // X向内缩进
        collisionRect.y + verticalPaddingTop, // Y从顶部向下缩进
        collisionRect.width - horizontalPadding, // 宽度减小
        collisionRect.height - (verticalPaddingTop + verticalPaddingBottom) // 高度减小
    };

    if (adjustedRect.width < 1.0f) adjustedRect.width = 1.0f;
    if (adjustedRect.height < 1.0f) adjustedRect.height = 1.0f;

    return adjustedRect;
}
