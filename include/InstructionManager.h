// InstructionManager.h
#ifndef INSTRUCTION_MANAGER_H
#define INSTRUCTION_MANAGER_H

#include <vector>
#include <string>
#include <map>
#include "InstructionText.h" // 包含 InstructionText 定义

struct InstructionData
{
    std::string id;
    std::string message;
    int fontSize;
    Color textColor;
    float displayDuration;
    float fallGravity;
    Vector2 desiredCenterPos;
    float triggerAtGameTime;
    bool triggeredThisSession;
    bool isMandatoryToShow;

    InstructionData(std::string id, std::string msg, int fs, Color clr,
                    const float dispDur, const float fallGrav, const Vector2 pos,
                    const float triggerTime,
                    const bool mandatory = true)
        : id(std::move(id)), message(std::move(msg)), fontSize(fs), textColor(clr),
          displayDuration(dispDur), fallGravity(fallGrav),
          desiredCenterPos(pos),
          triggerAtGameTime(triggerTime),
          triggeredThisSession(false),
          isMandatoryToShow(mandatory)
    {
    }
};

class InstructionManager
{
public:
    InstructionManager();
    std::vector<Rectangle> GetAllActiveCollidableInstructionRects() const;
    void Initialize(int virtualScreenWidth, float groundY, const Sound& bombSfx);
    void Update(float deltaTime, float worldScrollSpeed, float currentGameTime);
    void Draw() const;
    void ResetAllInstructions();

private:
    std::map<std::string, InstructionData> instructionConfigs;
    std::vector<InstructionText> activeInstructionTexts;
    int screenWidthRef;
    float groundYRef;
    Sound bombSoundRef;
};

#endif // INSTRUCTION_MANAGER_H
