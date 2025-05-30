// include/InstructionManager.h
#ifndef INSTRUCTION_MANAGER_H
#define INSTRUCTION_MANAGER_H

#include <vector>
#include <string>
#include <map>
#include "InstructionText.h"

struct InstructionData
{
    std::string id = ""; // 唯一标识符
    std::string message = ""; // 显示的文本内容
    int fontSize = 12; // 字体大小，给个合理默认值
    Color textColor = WHITE; // 文本颜色，示例默认白色
    float displayDuration = 0.0f; // 文本固定显示时长
    float fallGravity = 0.0f; // 掉落时的重力
    Vector2 desiredCenterPos = {0, 0}; // 期望的文本中心位置
    float triggerAtGameTime = 0.0f; // 在游戏进行到此时间点时触发
    bool triggeredThisSession = false; // 本局游戏是否已触发过
    bool isMandatoryToShow = true; // 是否必须显示 (默认true)
};

// 管理游戏中所有教学提示的显示逻辑
class InstructionManager
{
public:
    InstructionManager();
    // 获取所有当前激活且可碰撞的教学文本的矩形区域
    std::vector<Rectangle> GetAllActiveCollidableInstructionRects() const;
    // 初始化教学管理器
    void Initialize(int virtualScreenWidth, float groundY, const Sound& bombSfx);
    // 更新所有教学文本的状态
    void Update(float deltaTime, float worldScrollSpeed, float currentGameTime);
    // 绘制所有激活的教学文本
    void Draw() const;
    // 重置所有教学提示的状态 (例如新游戏开始时)
    void ResetAllInstructions();

private:
    std::map<std::string, InstructionData> instructionConfigs; // 存储所有教学配置的映射表 (ID -> Data)
    std::vector<InstructionText> activeInstructionTexts; // 当前屏幕上激活的教学文本列表
    int screenWidthRef; // 屏幕宽度参考 (用于布局)
    float groundYRef; // 地面Y坐标参考 (用于教学文本掉落)
    Sound bombSoundRef; // 爆炸音效参考 (用于教学文本消失效果)
};

#endif // INSTRUCTION_MANAGER_H
