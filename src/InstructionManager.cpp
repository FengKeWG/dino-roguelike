// src/InstructionManager.cpp
#include "../include/InstructionManager.h"
#include "raylib.h"
#include <algorithm>

InstructionManager::InstructionManager()
    : screenWidthRef(0), groundYRef(0.0f), bombSoundRef{nullptr}
{
}

// 初始化
void InstructionManager::Initialize(const int virtualScreenWidth, const float groundY, const Sound& bombSfx)
{
    screenWidthRef = virtualScreenWidth;
    groundYRef = groundY;
    bombSoundRef = bombSfx;
    instructionConfigs.clear();
    activeInstructionTexts.clear();
    instructionConfigs.emplace("jump_tip",
                               InstructionData("jump_tip", "Press SPACE or W to JUMP", 24, DARKGRAY,
                                               2.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 100.0f},
                                               2.0f,
                                               true));
    instructionConfigs.emplace("move_tip",
                               InstructionData("move_tip", "Press A or D to MOVE", 24, DARKGRAY,
                                               2.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 100.0f},
                                               6.0f,
                                               true));
    instructionConfigs.emplace("sneak_tip",
                               InstructionData("sneak_tip", "Press S to SNEAK", 24, DARKGRAY,
                                               2.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 100.0f},
                                               10.0f,
                                               true));
    instructionConfigs.emplace("dash_tip",
                               InstructionData("dash_tip", "Press Shift to DASH", 24, DARKGRAY,
                                               2.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 100.0f},
                                               14.0f,
                                               true));
    instructionConfigs.emplace("attack_tip",
                               InstructionData("attack_tip", "Left Click to ATTACK", 24, DARKGRAY,
                                               2.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 100.0f},
                                               18.0f,
                                               true));
    instructionConfigs.emplace("good",
                               InstructionData("good", "GOOD LUCK and HAVE FUN :)", 24, DARKGRAY,
                                               2.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, 100.0f},
                                               22.0f,
                                               true));
    instructionConfigs.emplace("60_1",
                               InstructionData("60_1", "GOOD JOB", 24, DARKGRAY,
                                               0.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 8.0f, -10.0f},
                                               60.0f,
                                               true));
    instructionConfigs.emplace("60_2",
                               InstructionData("60_2", "You", 24, DARKGRAY,
                                               0.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 4.0f, -10.0f},
                                               60.5f,
                                               true));
    instructionConfigs.emplace("60_3",
                               InstructionData("60_3", "Made It", 24, DARKGRAY,
                                               0.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) / 2.0f, -10.0f},
                                               61.0f,
                                               true));
    instructionConfigs.emplace("60_4",
                               InstructionData("60_4", "Through 60 Seconds", 24, DARKGRAY,
                                               0.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) * 3 / 4.0f, -10.0f},
                                               61.5f,
                                               true));
    instructionConfigs.emplace("60_5",
                               InstructionData("60_5", "GO ON!", 24, DARKGRAY,
                                               0.0f, 1600.0f,
                                               {static_cast<float>(virtualScreenWidth) * 4 / 5.0f, -10.0f},
                                               62.0f,
                                               true));
    ResetAllInstructions();
}

void InstructionManager::Update(const float deltaTime, const float worldScrollSpeed, const float currentGameTime)
{
    for (auto& instructionText : activeInstructionTexts)
    {
        instructionText.Update(deltaTime, worldScrollSpeed);
    }
    activeInstructionTexts.erase(
        std::ranges::remove_if(activeInstructionTexts,
                               [](const InstructionText& it) { return it.IsDone(); }).begin(),
        activeInstructionTexts.end());
    for (auto& pair : instructionConfigs)
    {
        // 如果当前游戏时间达到触发时间，并且该提示本局游戏尚未触发
        if (InstructionData& data = pair.second; currentGameTime >= data.triggerAtGameTime && !data.
            triggeredThisSession)
        {
            InstructionText newText;
            newText.Initialize(
                data.message.c_str(), data.fontSize, data.textColor,
                data.displayDuration, data.fallGravity,
                screenWidthRef, groundYRef, bombSoundRef
            );
            newText.Activate(data.desiredCenterPos); // 激活文本，并设置其初始位置
            activeInstructionTexts.push_back(newText); // 将新的文本添加到激活列表中
            data.triggeredThisSession = true;
        }
    }
}

void InstructionManager::Draw() const
{
    for (const auto& instructionText : activeInstructionTexts)
    {
        instructionText.Draw();
    }
}

void InstructionManager::ResetAllInstructions()
{
    activeInstructionTexts.clear();
    for (auto& pair : instructionConfigs)
    {
        pair.second.triggeredThisSession = false;
    }
}

// 获取所有当前激活且可碰撞的教学文本的矩形区域
std::vector<Rectangle> InstructionManager::GetAllActiveCollidableInstructionRects() const
{
    std::vector<Rectangle> collidableRects;
    for (const auto& activeText : activeInstructionTexts)
    {
        InstructionTextState state = activeText.GetCurrentState();
        if (state == InstructionTextState::DISPLAYING || state == InstructionTextState::FALLING)
        {
            collidableRects.push_back(activeText.GetCollisionRect());
        }
    }
    return collidableRects;
}
