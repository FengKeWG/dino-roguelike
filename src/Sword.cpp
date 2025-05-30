// src/Sword.cpp
#include "../include/Sword.h"

Sword::Sword(const Texture2D& tex, const Sound& sound, Dinosaur* ownerDino)
    : texture(tex),
      swingSound(sound),
      owner(ownerDino),
      cooldownTimer(0.0f),
      attackCooldown(1.0f),
      isAttackingState(false),
      attackTimer(0.0f),
      attackDuration(0.3f),
      currentVisualRotation(0.0f),
      pivotInTexture({25.0f, 150.0f}), // 旋转轴心点
      drawScale(0.3f),
      textureInitialAngle(-45.0f), // 纹理本身的初始角度
      swingStartAngleWorld(-75.0f), // 挥动开始时的世界角度
      swingEndAngleWorld(75.0f) // 挥动结束时的世界角度
{
}

Sword::~Sword() = default;

bool Sword::IsAttacking() const
{
    return isAttackingState;
}

// 获取剑在恐龙身上的旋转中心点
Vector2 Sword::GetAttachmentPoint() const
{
    if (owner->IsFacingRight()) // 恐龙朝右
    {
        return {owner->position.x + owner->GetWidth() * 0.75f, owner->position.y + owner->GetHeight() * 0.40f};
    }
    // 恐龙朝左
    return {owner->position.x + owner->GetWidth() * 0.25f, owner->position.y + owner->GetHeight() * 0.40f};
}

// 执行攻击
void Sword::Attack()
{
    if (!isAttackingState && cooldownTimer <= 0.0f)
    {
        isAttackingState = true;
        attackTimer = 0.0f; // 重置攻击动画计时器
        cooldownTimer = attackCooldown; // 开始攻击冷却
        PlaySound(swingSound);
    }
}

// 更新剑的状态，每帧调用
void Sword::Update(const float deltaTime)
{
    // 更新冷却计时器
    if (cooldownTimer > 0.0f)
    {
        cooldownTimer -= deltaTime;
        if (cooldownTimer < 0.0f) cooldownTimer = 0.0f;
    }

    if (!isAttackingState) return;

    attackTimer += deltaTime;
    float attackProgress = attackTimer / attackDuration; // 计算攻击动画进度 (0.0 到 1.0)

    // 如果攻击动画完成
    if (attackProgress >= 1.0f)
    {
        isAttackingState = false;
        attackProgress = 1.0f;
    }

    // 使用线性插值lerp根据攻击进度计算剑当前的相对世界角度
    const float currentRelativeAngle = Lerp(swingStartAngleWorld, swingEndAngleWorld, attackProgress);
    float finalDrawRotation; // 最终用于绘制的旋转角度

    if (owner->IsFacingRight())
    {
        finalDrawRotation = currentRelativeAngle - textureInitialAngle;
    }
    else
    {
        // 计算朝左时的目标世界角度
        const float targetWorldAngleLeft = 180.0f - currentRelativeAngle;
        // 计算朝左时纹理的初始角度
        const float flippedTextureInitialAngle = 180.0f - textureInitialAngle;
        // 计算最终绘制角度
        finalDrawRotation = targetWorldAngleLeft - flippedTextureInitialAngle;
    }
    currentVisualRotation = finalDrawRotation;
}

// 绘制剑
void Sword::Draw() const
{
    if (!isAttackingState) return;

    // 源矩形 
    Rectangle sourceRec = {0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
    const auto [x, y] = GetAttachmentPoint(); // 剑的附着点

    Vector2 drawOrigin = {pivotInTexture.x * drawScale, pivotInTexture.y * drawScale};

    if (!owner->IsFacingRight())
    {
        sourceRec.width *= -1; // 翻转
        drawOrigin.x = (texture.width - pivotInTexture.x) * drawScale;
    }

    const Rectangle destRec = {
        x,
        y,
        std::fabs(sourceRec.width) * drawScale,
        sourceRec.height * drawScale
    };

    DrawTexturePro(texture, sourceRec, destRec, drawOrigin, currentVisualRotation, WHITE);
}

// 获取剑的轴对齐包围盒AABB
Rectangle Sword::GetSwordAABB() const
{
    const auto [x, y] = GetAttachmentPoint();
    const float baseSwordWidth = texture.width * drawScale;
    const float baseSwordHeight = texture.height * drawScale;
    float topLeftX; // AABB的左上角X
    const float topLeftY = y - (pivotInTexture.y * drawScale); // AABB的左上角Y (基于轴心点)

    // 根据朝向计算AABB的左上角X
    if (owner->IsFacingRight())
    {
        topLeftX = x - (pivotInTexture.x * drawScale);
    }
    else // 朝左时，轴心点相对于纹理右侧
    {
        topLeftX = x - ((texture.width - pivotInTexture.x) * drawScale);
    }

    // 未旋转时的基础AABB
    const Rectangle aabb = {topLeftX, topLeftY, baseSwordWidth, baseSwordHeight};

    // 对AABB进行缩减，得到一个更小、更符合剑刃实际攻击范围的碰撞框
    constexpr float collisionWidthScale = 0.6f; // 宽度缩减到60%
    constexpr float collisionHeightScale = 0.4f; // 高度缩减到40%
    const float reducedWidth = aabb.width * collisionWidthScale;
    const float reducedHeight = aabb.height * collisionHeightScale;
    const float offsetX = (aabb.width - reducedWidth); // X方向的偏移，使缩减后的矩形居中
    const float offsetY = (aabb.height - reducedHeight) * 0.1f; // Y方向的偏移，主要向上移动，取剑的前端

    Rectangle collisionRect = {
        aabb.x + offsetX, // 调整后的X
        aabb.y + offsetY, // 调整后的Y
        reducedWidth, // 缩减后的宽度
        reducedHeight // 缩减后的高度
    };

    // 确保碰撞框的宽高不小于1
    if (collisionRect.width < 1.0f) collisionRect.width = 1.0f;
    if (collisionRect.height < 1.0f) collisionRect.height = 1.0f;
    return collisionRect;
}

// 检测剑与鸟群的碰撞
void Sword::CheckCollisionsWithBirds(std::vector<Bird>& birds, int& gameScore,
                                     ParticleSystem& effectParticles,
                                     const ParticleProperties& effectProps,
                                     const float worldScrollSpeed, const Sound& birdScreamSound) const
{
    if (!isAttackingState) return;
    const Rectangle swordRect = GetSwordAABB();
    if (swordRect.width <= 0 || swordRect.height <= 0) return;

    for (auto it = birds.begin(); it != birds.end();)
    {
        // 检查剑的碰撞框与鸟的碰撞框是否相交
        if (CheckCollisionRecs(swordRect, it->GetCollisionRect()))
        {
            PlaySound(birdScreamSound);
            const Vector2 birdCenter = {
                it->getPosition().x + it->GetWidth() / 2.0f,
                it->getPosition().y + it->GetHeight() / 2.0f
            };
            // 血液粒子效果
            effectParticles.Emit(birdCenter, GetRandomValue(25, 40), effectProps, worldScrollSpeed);
            it = birds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool Sword::IsOnCooldown() const
{
    return cooldownTimer > 0.0f;
}

// 获取冷却进度
float Sword::GetCooldownProgress() const
{
    if (attackCooldown <= 0.0f || cooldownTimer <= 0.0f)
    {
        return 0.0f;
    }
    return cooldownTimer / attackCooldown;
}
