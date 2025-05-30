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
    position = {startX, groundY - runHeight};
    UpdateCollisionRect();
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
        if (!IsOnGround())
        {
            velocity.y += gravity * deltaTime;
        }
        position.y += velocity.y * deltaTime;
        if (IsOnGround())
        {
            position.y = (groundY - GetHeight()) + 5.0f;
            velocity.y = 0;
        }
        UpdateCollisionRect();
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
            const int particlesToEmit = randI(2, 4);
            for (int i = 0; i < particlesToEmit; ++i)
            {
                const float dinoWidth = GetWidth();
                const float dinoHeight = GetHeight();
                const Vector2 particleEmitPos = {
                    position.x + randF(dinoWidth * 0.1f, dinoWidth * 0.9f),
                    position.y + randF(dinoHeight * 0.1f, dinoHeight * 0.9f)
                };
                if (dashDirection.x > 0)
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
            currentEffectiveGravity *= sneakGravityMultiplier;
        }
        if (!IsOnGround())
        {
            velocity.y += currentEffectiveGravity * deltaTime;
        }
    }
    position.y += velocity.y * deltaTime;
    if (IsOnGround())
    {
        if (velocity.y >= 0)
        {
            velocity.y = 0;
            position.y = (groundY - GetHeight()) + 5.0f;
            if (isJumping)
            {
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
        isJumping = true;
    }
    if (const std::vector<Texture2D>* currentFrames = GetCurrentAnimationFramesPointer(); currentFrames && !
        currentFrames->empty())
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
    return (position.y + GetHeight() >= groundY - 5.0f);
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
    isJumping = true;
    coyoteTimeCounter = 0.0f;
    jumpQueued = false;
    jumpBufferCounter = 0.0f;
    currentAnimFrameIndex = 0;
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
        if ((isJumping || isDashing) && !isSneaking)
        {
            if (!runFrames.empty() && runFrames[0].id > 0)
            {
                return runFrames[0];
            }
        }
        else if ((isJumping || isDashing) && isSneaking)
        {
            if (!sneakFrames.empty() && sneakFrames[0].id > 0)
            {
                return sneakFrames[0];
            }
        }
        if (frameIdxToUse >= 0 && frameIdxToUse < frames.size() && frames[frameIdxToUse].id > 0)
        {
            return frames[frameIdxToUse];
        }

        if (!frames.empty() && frames[0].id > 0)
        {
            return frames[0];
        }
    }
    if (!runFrames.empty() && runFrames[0].id > 0)
    {
        return runFrames[0];
    }
    return Texture2D{};
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
