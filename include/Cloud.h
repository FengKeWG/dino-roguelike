// include/Cloud.h
#ifndef CLOUD_H
#define CLOUD_H

#include "raylib.h"

class Cloud
{
public:
    Cloud(Texture2D tex, Vector2 initialPosition, float spd);
    ~Cloud();

    void Update(float deltaTime);
    void Draw() const;
    bool IsOffScreen() const; // 检查云是否移出屏幕左侧

    // 如果需要从外部访问，可以添加getter，但目前来看，内部逻辑足够
    // Vector2 GetPosition() const { return position; }
    // float GetSpeed() const { return speed; }

private:
    Texture2D texture;
    Vector2 position;
    float speed;
};

#endif // CLOUD_H
