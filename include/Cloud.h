// include/Cloud.h
#ifndef CLOUD_H
#define CLOUD_H

#include "raylib.h"

class Cloud
{
public:
    Cloud(const Texture2D& tex, Vector2 initialPosition, float spd);
    ~Cloud();

    // 更新云彩状态
    void Update(float deltaTime);
    // 绘制云彩
    void Draw() const;
    // 检查云彩是否移出屏幕
    bool IsOffScreen() const;

private:
    Texture2D texture; // 云彩纹理
    Vector2 position; // 云彩位置
    float speed; // 云彩移动速度
};

#endif // CLOUD_H
