#include "../include/Sword.h"

Sword::Sword(Texture2D tex, Sound sound, Dinosaur* ownerDino)
    : texture(tex),
      swingSound(sound),
      owner(ownerDino),
      isAttackingState(false),
      attackTimer(0.0f),
      attackDuration(0.3f),
      currentVisualRotation(0.0f),
      pivotInTexture({25.0f, 150.0f}),
      drawScale(0.3f),
      textureInitialAngle(-45.0f),
      swingStartAngleWorld(-75.0f),
      swingEndAngleWorld(75.0f),
      cooldownTimer(0.0f),
      attackCooldown(0.0f)
{
    if (!owner)
    {
        TraceLog(LOG_ERROR, "错误：剑在创建时没有指定主人 (Dinosaur)!");
    }
}

Sword::~Sword()
{
}

bool Sword::IsAttacking() const
{
    return isAttackingState;
}


Vector2 Sword::GetAttachmentPoint() const
{
    if (!owner) return {0, 0};


    if (owner->IsFacingRight())
    {
        return {owner->position.x + owner->GetWidth() * 0.75f, owner->position.y + owner->GetHeight() * 0.40f};
    }
    else
    {
        return {owner->position.x + owner->GetWidth() * 0.25f, owner->position.y + owner->GetHeight() * 0.40f};
    }
}

void Sword::Attack()
{
    if (!isAttackingState && owner && cooldownTimer <= 0.0f)
    {
        TraceLog(LOG_INFO, "Sword::Attack() - Conditions met, starting attack.");
        isAttackingState = true;
        attackTimer = 0.0f;

        if (swingSound.frameCount > 0 && IsAudioDeviceReady())
        {
            PlaySound(swingSound);
        }
    }
    else
    {
        if (cooldownTimer > 0.0f)
        {
        }
    }
}

void Sword::Update(float deltaTime)
{
    if (cooldownTimer > 0.0f)
    {
        cooldownTimer -= deltaTime;
    }

    if (!isAttackingState || !owner) return;

    attackTimer += deltaTime;
    float attackProgress = attackTimer / attackDuration;

    if (attackProgress >= 1.0f)
    {
        isAttackingState = false;
        attackProgress = 1.0f;
        cooldownTimer = attackCooldown;
    }


    float currentRelativeAngle = Lerp(swingStartAngleWorld, swingEndAngleWorld, attackProgress);
    float finalDrawRotation;

    if (owner->IsFacingRight())
    {
        finalDrawRotation = currentRelativeAngle - textureInitialAngle;
    }
    else
    {
        float targetWorldAngleLeft = 180.0f - currentRelativeAngle;

        float flippedTextureInitialAngle = 180.0f - textureInitialAngle;

        finalDrawRotation = targetWorldAngleLeft - flippedTextureInitialAngle;
    }
    currentVisualRotation = finalDrawRotation;
}

void Sword::Draw() const
{
    if (!isAttackingState || !owner || texture.id == 0) return;

    Rectangle sourceRec = {0.0f, 0.0f, (float)texture.width, (float)texture.height};
    Vector2 attachPoint = GetAttachmentPoint();


    Vector2 drawOrigin = {pivotInTexture.x * drawScale, pivotInTexture.y * drawScale};

    if (!owner->IsFacingRight())
    {
        sourceRec.width *= -1;

        drawOrigin.x = (texture.width - pivotInTexture.x) * drawScale;
    }

    Rectangle destRec = {
        attachPoint.x,
        attachPoint.y,
        std::fabs(sourceRec.width) * drawScale,
        sourceRec.height * drawScale
    };

    DrawTexturePro(texture, sourceRec, destRec, drawOrigin, currentVisualRotation, WHITE);
}


Rectangle Sword::GetSwordAABB() const
{
    if (!owner || texture.id == 0 || !isAttackingState) return {0, 0, 0, 0};

    Vector2 attachPoint = GetAttachmentPoint();
    float baseSwordWidth = texture.width * drawScale;
    float baseSwordHeight = texture.height * drawScale;

    float topLeftX;
    float topLeftY = attachPoint.y - (pivotInTexture.y * drawScale);

    if (owner->IsFacingRight())
    {
        topLeftX = attachPoint.x - (pivotInTexture.x * drawScale);
    }
    else
    {
        topLeftX = attachPoint.x - ((texture.width - pivotInTexture.x) * drawScale);
    }

    Rectangle aabb = {topLeftX, topLeftY, baseSwordWidth, baseSwordHeight};


    float collisionWidthScale = 0.6f;
    float collisionHeightScale = 0.4f;

    float reducedWidth = aabb.width * collisionWidthScale;
    float reducedHeight = aabb.height * collisionHeightScale;


    float offsetX = (aabb.width - reducedWidth);

    float offsetY = (aabb.height - reducedHeight) * 0.1f;


    Rectangle collisionRect = {
        aabb.x + offsetX,
        aabb.y + offsetY,
        reducedWidth,
        reducedHeight
    };

    if (collisionRect.width < 1.0f) collisionRect.width = 1.0f;
    if (collisionRect.height < 1.0f) collisionRect.height = 1.0f;


    return collisionRect;
}

void Sword::CheckCollisionsWithBirds(std::vector<Bird>& birds, int& gameScore,
                                     ParticleSystem& effectParticles,
                                     const ParticleProperties& effectProps,
                                     float worldScrollSpeed, const Sound& birdScreamSound)
{
    if (!isAttackingState) return;

    Rectangle swordRect = GetSwordAABB();
    if (swordRect.width <= 0 || swordRect.height <= 0) return;

    for (auto it = birds.begin(); it != birds.end(); /* 在循环内部增量 */)
    {
        if (CheckCollisionRecs(swordRect, it->GetCollisionRect()))
        {
            if (birdScreamSound.frameCount > 0 && IsAudioDeviceReady())
            {
                PlaySound(birdScreamSound);
            }

            Vector2 birdCenter = {
                it->getPosition().x + it->GetWidth(), it->getPosition().y + it->GetHeight()
            };

            effectParticles.Emit(birdCenter, GetRandomValue(25, 40), effectProps, worldScrollSpeed);

            it = birds.erase(it);
            gameScore += 50;
        }
        else
        {
            ++it;
        }
    }
}
