// InstructionManager.cpp
#include "../include/InstructionManager.h"
#include "raylib.h" // For TraceLog, etc.

InstructionManager::InstructionManager()
    : screenWidthRef(0), groundYRef(0.0f), bombSoundRef{0},
      currentActiveInstructionId("")
{
    // activeInstructionText 会被默认构造
}

void InstructionManager::Initialize(int virtualScreenWidth, float groundY, Sound bombSfx)
{
    screenWidthRef = virtualScreenWidth;
    groundYRef = groundY;
    bombSoundRef = bombSfx;

    instructionConfigs.clear(); // 清除旧配置

    // 在这里定义所有的教学提示
    // 提示1: 跳跃
    instructionConfigs.emplace("jump_tip",
                               InstructionData("jump_tip", "Press SPACE or W to JUMP", 24, DARKGRAY,
                                               2.5f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 80.0f}, // 期望中心位置
                                               2.0f, // 延迟2秒激活
                                               true)); // 必须显示

    // 提示2: 移动
    instructionConfigs.emplace("move_tip",
                               InstructionData("move_tip", "Press A or D to MOVE", 24, DARKBLUE,
                                               3.0f, 1500.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 120.0f},
                                               5.0f, // 在被请求后延迟0.5秒激活 (这个延迟可以由Game类控制何时Request)
                                               true));

    // 提示3: BOSS出现 (示例)
    instructionConfigs.emplace("boss_warning",
                               InstructionData("boss_warning", "BOSS APPROACHING!", 32, RED,
                                               4.0f, 1200.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 100.0f},
                                               0.0f, // 立即显示
                                               true)); // 假设BOSS警告总是显示

    // ...可以添加更多提示...

    ResetAllInstructions(); // 确保初始状态正确
}

void InstructionManager::RequestShowInstruction(const std::string& instructionId)
{
    if (isAnyInstructionBeingProcessed())
    {
        // TraceLog(LOG_INFO, "InstructionManager: Another instruction ('%s') is active, cannot show '%s' yet.",
        //          currentActiveInstructionId.c_str(), instructionId.c_str());
        return; // 如果当前有提示正在处理，则不立即显示新的（可以加入队列或稍后重试逻辑）
    }

    auto it = instructionConfigs.find(instructionId);
    if (it != instructionConfigs.end())
    {
        InstructionData& data = it->second;
        if (!data.triggeredThisSession || !data.isMandatoryToShow)
        {
            // 如果未触发过，或者非强制但允许重复
            // TraceLog(LOG_INFO, "InstructionManager: Requesting to show instruction '%s'.", instructionId.c_str());
            data.activationDelayPhaseActive = true;
            data.currentDelayTimer = 0.0f;
            // data.triggeredThisSession = true; // 应该在实际激活时设置
        }
        else
        {
            // TraceLog(LOG_INFO, "InstructionManager: Instruction '%s' already triggered this session or not mandatory.", instructionId.c_str());
        }
    }
    else
    {
        TraceLog(LOG_WARNING, "InstructionManager: Unknown instructionId '%s' requested.", instructionId.c_str());
    }
}

bool InstructionManager::isAnyInstructionBeingProcessed() const
{
    return !currentActiveInstructionId.empty() || activeInstructionText.IsActive();
}


void InstructionManager::Update(float deltaTime)
{
    // 1. 更新当前活跃的 InstructionText 对象
    if (activeInstructionText.IsActive())
    {
        activeInstructionText.Update(deltaTime);
        if (activeInstructionText.IsDone())
        {
            // TraceLog(LOG_INFO, "InstructionManager: Active instruction '%s' is done.", currentActiveInstructionId.c_str());
            if (!currentActiveInstructionId.empty())
            {
                auto it = instructionConfigs.find(currentActiveInstructionId);
                if (it != instructionConfigs.end())
                {
                    // 可以在这里标记它真的完成了，或者 Game 类通过 IsInstructionDone 查询
                }
            }
            currentActiveInstructionId = ""; // 清除当前活跃ID
            // activeInstructionText.Reset(); // Reset它以便下次使用
        }
    }

    // 2. 遍历所有配置，处理处于延迟激活阶段的提示
    //    并且当前没有其他指令正在播放/处理
    if (!isAnyInstructionBeingProcessed())
    {
        for (auto& pair : instructionConfigs)
        {
            InstructionData& data = pair.second;
            if (data.activationDelayPhaseActive && !data.triggeredThisSession)
            {
                data.currentDelayTimer += deltaTime;
                if (data.currentDelayTimer >= data.activationDelay)
                {
                    // TraceLog(LOG_INFO, "InstructionManager: Activating instruction '%s'.", data.id.c_str());

                    activeInstructionText.Initialize(
                        data.message.c_str(), data.fontSize, data.textColor,
                        data.displayDuration, data.fallGravity,
                        screenWidthRef, groundYRef, bombSoundRef
                    );
                    activeInstructionText.Activate(data.desiredCenterPos);

                    currentActiveInstructionId = data.id;
                    data.triggeredThisSession = true; // 标记已触发
                    data.activationDelayPhaseActive = false; // 结束延迟阶段
                    break; // 一次只激活一个
                }
            }
        }
    }
}

void InstructionManager::Draw() const
{
    if (activeInstructionText.IsActive())
    {
        activeInstructionText.Draw();
    }
}

void InstructionManager::ResetAllInstructions()
{
    activeInstructionText.Reset();
    currentActiveInstructionId = "";
    for (auto& pair : instructionConfigs)
    {
        pair.second.triggeredThisSession = false;
        pair.second.activationDelayPhaseActive = false;
        pair.second.currentDelayTimer = 0.0f;
    }
}

bool InstructionManager::IsInstructionDone(const std::string& instructionId) const
{
    if (currentActiveInstructionId == instructionId)
    {
        return activeInstructionText.IsDone();
    }
    // 如果不是当前活跃的，但之前触发过，我们也可以认为它“完成”了显示过程
    auto it = instructionConfigs.find(instructionId);
    if (it != instructionConfigs.end())
    {
        return it->second.triggeredThisSession && !activeInstructionText.IsActive(); // 如果触发过且当前没有活跃文本
    }
    return false;
}

bool InstructionManager::IsInstructionActive(const std::string& instructionId) const
{
    if (currentActiveInstructionId == instructionId)
    {
        return activeInstructionText.IsActive();
    }
    return false; // 或者检查 instructionConfigs 中对应ID的 activationDelayPhaseActive
}
