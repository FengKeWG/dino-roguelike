// include/Cloud.h
#ifndef CLOUD_H
#define CLOUD_H

#include "raylib.h"

class Cloud
{
public:
    Cloud(const Texture2D& tex, Vector2 initialPosition, float spd);
    ~Cloud();

    void Update(float deltaTime);
    void Draw() const;
    bool IsOffScreen() const;

private:
    Texture2D texture;
    Vector2 position;
    float speed;
};

#endif // CLOUD_H
