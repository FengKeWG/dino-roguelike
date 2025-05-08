#ifndef DINOSAUR_H
#define DINOSAUR_H

#include "raylib.h"
#include <vector>

// 状态不再使用枚举，用布尔值更灵活
// enum class DinoActionState { RUNNING, JUMPING, SNEAKING };

class Dinosaur
{
public:
    Vector2 position;
    Vector2 velocity; // Y方向速度用于跳跃
    float gravity;
    float jumpSpeed;
    float moveSpeed; // 水平移动速度

    Sound jumpSoundHandle;

    bool isJumping; // 是否在空中（由速度和地面检测判断）
    bool isSneaking; // Shift 键是否按下
    bool facingRight; // 图像朝向

    Rectangle collisionRect; // 原始碰撞盒（基于完整纹理）

    // 动画帧纹理
    std::vector<Texture2D> runFrames; // 跑动帧，跳跃时也用此动画的第一帧
    std::vector<Texture2D> sneakFrames; // 下蹲帧

    int currentAnimFrameIndex; // 当前播放的动画帧索引
    float frameTimeCounter; // 动画帧计时器
    float animationSpeed; // 动画帧切换速率（秒/帧）

    float groundY; // 地面Y坐标
    float runHeight; // 跑动/站立时的固定高度
    float sneakHeight; // 下蹲时的固定高度

    // --- 跳跃增强机制 ---
    float coyoteTimeCounter; // 土狼时间计时器: 离开地面后仍可跳跃的时间
    const float coyoteTimeDuration = 0.1f; // 土狼时间持续秒数 (可调)

    float jumpBufferCounter; // 跳跃缓冲计时器: 落地前按下跳跃的有效时间
    const float jumpBufferDuration = 0.12f; // 跳跃缓冲持续秒数 (可调)
    bool jumpQueued; // 是否有一个跳跃指令在排队等待执行

    // 构造函数 (移除 idleTex, jumpTex)
    Dinosaur(float startX, float initialGroundY,
             const std::vector<Texture2D>& runTex,
             const std::vector<Texture2D>& sneakTex,
             Sound jumpSound);
    ~Dinosaur();

    void RequestJump(); // 玩家请求跳跃 (由Game类在输入时调用)
    void StartSneaking(); // 切换到下蹲姿态
    void StopSneaking(); // 恢复跑动姿态
    void Move(float direction, float deltaTime); // 处理左右移动输入
    void Update(float deltaTime); // 更新状态、物理、动画
    void Draw(); // 绘制恐龙

    Rectangle GetCollisionRect() const; // 获取用于碰撞检测的调整后矩形
    float GetWidth() const; // 获取当前姿态的宽度
    float GetHeight() const; // 获取当前姿态的高度
    void UpdateCollisionRect(); // 更新原始 collisionRect 成员
    bool IsOnGround() const; // 辅助判断是否在地面\
    
private:
    void ExecuteJump(); // 实际执行跳跃的私有方法
    Texture2D GetCurrentTextureToDraw() const; // 获取当前应绘制的纹理
    const std::vector<Texture2D>* GetCurrentAnimationFramesPointer() const; // 获取当前动画帧组指针
};

#endif // DINOSAUR_H
