#include "../include/Obstacle.hpp"
#include <iostream>

Obstacle::Obstacle(float startX, float groundYPos, float obstacleSpeed, Texture2D tex)
    : speed(obstacleSpeed), texture(tex) {
    if (texture.id == 0) {
        std::cerr << "Warning: Obstacle texture not loaded!" << std::endl;
        // 默认值以防万一
        position = {startX, groundYPos - 50.0f};
        collisionRect = {position.x, position.y, 20, 50};
    } else {
        // position.y 是障碍物的顶部 Y 坐标
        position = {startX, groundYPos - static_cast<float>(texture.height)};
        collisionRect = {
            position.x,
            position.y,
            static_cast<float>(texture.width),
            static_cast<float>(texture.height)
        };
    }
}

Obstacle::~Obstacle() {
    // 纹理的加载和卸载由 Game 类管理
}

void Obstacle::Update(float deltaTime) {
    position.x -= speed * deltaTime;
    UpdateCollisionRect();
}

void Obstacle::Draw() {
    if (texture.id > 0) {
        DrawTexture(texture, static_cast<int>(position.x), static_cast<int>(position.y), WHITE);
    } else {
        DrawRectangleRec(GetCollisionRect(), RED); // 备用绘制
    }
    // DrawRectangleLinesEx(GetCollisionRect(), 1, BLUE); // 调试碰撞框
}

void Obstacle::UpdateCollisionRect() {
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    if (texture.id > 0) {
        collisionRect.width = static_cast<float>(texture.width);
        collisionRect.height = static_cast<float>(texture.height);
    }
}

Rectangle Obstacle::GetCollisionRect() const {
    return collisionRect;
}

bool Obstacle::IsOffScreen(float /*screenWidth*/) const {
    // screenWidth 参数暂时不用，因为只判断左边
    // 检查障碍物是否完全移出屏幕左侧
    return (position.x + GetWidth()) < 0;
}

float Obstacle::GetWidth() const {
    if (texture.id > 0) {
        return static_cast<float>(texture.width);
    }
    return 20.0f; // Default
}
