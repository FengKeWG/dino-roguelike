// InstructionManager.h
#ifndef INSTRUCTION_MANAGER_H
#define INSTRUCTION_MANAGER_H

#include <vector>
#include <string>
#include <map>
#include "InstructionText.h" // 包含 InstructionText 定义
// #include "InstructionData.h" // 如果 InstructionData 在单独文件

// InstructionData 结构体可以放在这里，或者单独的头文件
// (为了简洁，先放在这里)
struct InstructionData
{
    std::string id;
    std::string message;
    int fontSize;
    Color textColor;
    float displayDuration;
    float fallGravity;
    float activationDelay;
    Vector2 desiredCenterPos;

    bool triggeredThisSession;
    bool activationDelayPhaseActive;
    float currentDelayTimer;
    bool isMandatoryToShow; // 是否必须显示（例如，第一次玩游戏）

    InstructionData(std::string _id, std::string _msg, int _fs, Color _clr,
                    float _dispDur, float _fallGrav, Vector2 _pos, float _actDelay = 0.0f, bool _mandatory = true)
        : id(std::move(_id)), message(std::move(_msg)), fontSize(_fs), textColor(_clr),
          displayDuration(_dispDur), fallGravity(_fallGrav), desiredCenterPos(_pos),
          activationDelay(_actDelay),
          triggeredThisSession(false), activationDelayPhaseActive(false), currentDelayTimer(0.0f),
          isMandatoryToShow(_mandatory)
    {
    }
};


class InstructionManager
{
public:
    InstructionManager();

    // 在游戏开始或需要时配置所有教学提示
    void Initialize(int virtualScreenWidth, float groundY, Sound bombSfx);

    // 外部事件可以调用这个来尝试触发一个提示
    // 例如：Game::UpdateGame() 中特定条件满足时调用 RequestShowInstruction("jump_tip");
    void RequestShowInstruction(const std::string& instructionId);

    void Update(float deltaTime, float worldScrollSpeed);
    void Draw() const;

    // 重置所有提示的状态（例如新游戏开始时）
    void ResetAllInstructions();

    // 检查某个特定提示是否已经完成其动画
    bool IsInstructionDone(const std::string& instructionId) const;

    // 检查某个特定提示当前是否活跃（显示、下落、爆炸）
    bool IsInstructionActive(const std::string& instructionId) const;

    // 新增方法：检查是否有活动的、可碰撞的提示
    bool IsAnyInstructionActiveAndCollidable() const;
    // 新增方法：获取活动提示的碰撞矩形
    Rectangle GetActiveInstructionCollisionRect() const;

private:
    std::map<std::string, InstructionData> instructionConfigs; // 存储所有提示的配置数据
    InstructionText activeInstructionText; // 当前活跃（正在显示/动画）的提示文本对象
    // 假设同一时间只显示一个教学提示

    std::string currentActiveInstructionId; // 当前正在使用 activeInstructionText 的提示ID

    // 用于初始化的共享参数
    int screenWidthRef;
    float groundYRef;
    Sound bombSoundRef;

    bool isAnyInstructionBeingProcessed() const;
};

#endif // INSTRUCTION_MANAGER_H
