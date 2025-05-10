#include "../include/Cloud.h"

Cloud::Cloud(const Texture2D& tex, const Vector2 initialPosition, const float spd)
    : texture(tex), position(initialPosition), speed(spd)
{
}

Cloud::~Cloud() = default;

void Cloud::Update(const float deltaTime)
{
    position.x -= speed * deltaTime;
}

void Cloud::Draw() const
{
    if (texture.id > 0)
    {
        DrawTextureV(texture, position, WHITE);
    }
}

bool Cloud::IsOffScreen() const
{
    return (position.x + texture.width < 0);
}
