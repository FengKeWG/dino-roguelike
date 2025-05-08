#include "../include/Game.h"
#include "raymath.h"
#include <iostream>
#include <cstdlib> // For rand(), srand()
#include <ctime>   // For time()

Game::Game(int width, int height, const char* title)
    : screenWidth(width), screenHeight(height),
      dino(nullptr), currentState(GameState::PLAYING),
      groundY(0), gameSpeed(300.0f), score(0),
      obstacleSpawnTimer(0.0f),
      minObstacleSpawnInterval(1.0f), maxObstacleSpawnInterval(2.5f),
      currentObstacleSpawnInterval(0.0f), // Will be set in InitGame/ResetGame
      totalRoadWidthCovered(0.0f)
{
    srand(static_cast<unsigned int>(time(nullptr))); // 初始化随机种子

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, title);
    SetWindowMinSize(virtualScreenWidth / 2, virtualScreenHeight / 2); // 最小物理窗口
    SetTargetFPS(60);

    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_POINT); // 或者 TEXTURE_FILTER_NEAREST
    /*
     *默认的纹理过滤方式（通常是双线性过滤 TEXTURE_FILTER_BILINEAR）会在像素之间进行插值，导致模糊效果
     *将纹理过滤方式改为 最近邻过滤 (TEXTURE_FILTER_POINT 或 TEXTURE_FILTER_NEAREST)。
     */

    LoadResources();
    InitGame(); // 初始化游戏状态和对象
    // 确保 HandleWindowResize 在一开始就被调用以正确设置所有依赖屏幕尺寸的变量
    HandleWindowResize();
}

Game::~Game()
{
    UnloadRenderTexture(targetRenderTexture); // 卸载渲染纹理
    UnloadResources();
    delete dino;
    CloseWindow();
}

void Game::LoadResources()
{
    Texture2D tempTex; // 用于临时接收加载的纹理

    // 清空之前的纹理，以防 ResetGame 时重复加载（虽然通常卸载后才加载）
    dinoRunFrames.clear();
    smallCactusTextures.clear();
    bigCactusTextures.clear();
    roadSegmentTextures.clear();

    // Dino
    tempTex = LoadTexture("assets/dino_run_0.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        dinoRunFrames.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/dino_run_0.png");
    }

    tempTex = LoadTexture("assets/dino_run_1.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        dinoRunFrames.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/dino_run_1.png");
    }
    // dino_run_2.png 已被 dino_jump.png 替代

    dinoJumpFrame = LoadTexture("assets/dino_jump.png");
    if (dinoJumpFrame.id > 0)
    {
        SetTextureFilter(dinoJumpFrame, TEXTURE_FILTER_POINT);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/dino_jump.png");
    }

    // Small Cacti
    tempTex = LoadTexture("assets/small_cactus_1.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        smallCactusTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/small_cactus_1.png");
    }

    tempTex = LoadTexture("assets/small_cactus_2.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        smallCactusTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/small_cactus_2.png");
    }

    tempTex = LoadTexture("assets/small_cactus_3.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        smallCactusTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/small_cactus_3.png");
    }

    // Big Cacti
    tempTex = LoadTexture("assets/big_cactus_1.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        bigCactusTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/big_cactus_1.png");
    }

    tempTex = LoadTexture("assets/big_cactus_2.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        bigCactusTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/big_cactus_2.png");
    }

    // Road Segments
    tempTex = LoadTexture("assets/road_1.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        roadSegmentTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/road_1.png");
    }

    tempTex = LoadTexture("assets/road_2.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        roadSegmentTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/road_2.png");
    }

    tempTex = LoadTexture("assets/road_3.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        roadSegmentTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/road_3.png");
    }

    tempTex = LoadTexture("assets/road_4.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        roadSegmentTextures.push_back(tempTex);
    }
    else
    {
        TraceLog(LOG_WARNING, "Failed to load texture: assets/road_4.png");
    }

    // 最终检查一下是否有关键纹理未加载成功，可以决定是否中止游戏或使用占位符
    if (dinoRunFrames.empty() || dinoJumpFrame.id == 0)
    {
        TraceLog(LOG_ERROR, "CRITICAL: Dinosaur core textures failed to load. Game might not work correctly.");
        // 在这里可以考虑抛出异常，或者设置一个标志位让游戏进入错误状态
    }
    if (roadSegmentTextures.empty())
    {
        TraceLog(LOG_ERROR, "CRITICAL: Road textures failed to load. Ground will not be visible.");
    }
}

void Game::UnloadResources()
{
    for (auto& tex : dinoRunFrames)
    {
        if (tex.id > 0) UnloadTexture(tex);
    }
    dinoRunFrames.clear();

    if (dinoJumpFrame.id > 0) UnloadTexture(dinoJumpFrame);
    dinoJumpFrame = {0}; // 重置为无效纹理

    for (auto& tex : smallCactusTextures)
    {
        if (tex.id > 0) UnloadTexture(tex);
    }
    smallCactusTextures.clear();

    for (auto& tex : bigCactusTextures)
    {
        if (tex.id > 0) UnloadTexture(tex);
    }
    bigCactusTextures.clear();

    for (auto& tex : roadSegmentTextures)
    {
        if (tex.id > 0) UnloadTexture(tex);
    }
    roadSegmentTextures.clear();
}

void Game::InitGame()
{
    // groundY 会在 HandleWindowResize 中设置，或基于初始 screenHeight
    // 确保在创建恐龙之前 groundY 是正确的
    this->groundY = static_cast<float>(virtualScreenHeight) * 0.85f; // 地面在屏幕85%的高度

    if (dino) delete dino;
    dino = new Dinosaur(80.0f, groundY, dinoRunFrames, dinoJumpFrame);

    obstacles.clear();
    score = 0;
    gameSpeed = 300.0f; // 初始速度
    obstacleSpawnTimer = 0.0f;
    // 设置一个初始的随机生成间隔
    currentObstacleSpawnInterval = minObstacleSpawnInterval +
        static_cast<float>(rand()) / (static_cast<float>(
            RAND_MAX / (maxObstacleSpawnInterval - minObstacleSpawnInterval)));


    InitRoadSegments(); // 初始化地面
    currentState = GameState::PLAYING;
}


void Game::InitRoadSegments()
{
    activeRoadSegments.clear();
    totalRoadWidthCovered = 0.0f;

    if (roadSegmentTextures.empty() || roadSegmentTextures[0].id == 0) return;

    // 使用 virtualScreenWidth
    while (totalRoadWidthCovered < virtualScreenWidth + roadSegmentTextures[0].width)
    {
        int randIdx = rand() % roadSegmentTextures.size();
        Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
        activeRoadSegments.push_back({chosenRoadTex, totalRoadWidthCovered});
        totalRoadWidthCovered += chosenRoadTex.width;
    }
}

void Game::UpdateRoadSegments(float deltaTime)
{
    if (roadSegmentTextures.empty()) return;
    totalRoadWidthCovered = 0;
    float rightmostX = 0;

    for (auto& segment : activeRoadSegments)
    {
        segment.xPosition -= gameSpeed * deltaTime;
        if (segment.xPosition + segment.texture.width > rightmostX)
        {
            rightmostX = segment.xPosition + segment.texture.width;
        }
    }
    totalRoadWidthCovered = rightmostX;

    while (!activeRoadSegments.empty() && activeRoadSegments.front().xPosition + activeRoadSegments.front().texture.
        width < 0)
    {
        activeRoadSegments.pop_front();
    }

    // 使用 virtualScreenWidth
    while (totalRoadWidthCovered < virtualScreenWidth + roadSegmentTextures[0].width * 2)
    {
        int randIdx = rand() % roadSegmentTextures.size();
        Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
        float newX = totalRoadWidthCovered;
        if (!activeRoadSegments.empty())
        {
            newX = activeRoadSegments.back().xPosition + activeRoadSegments.back().texture.width;
        }
        else
        {
            newX = 0;
        }
        activeRoadSegments.push_back({chosenRoadTex, newX});
        totalRoadWidthCovered = newX + chosenRoadTex.width;
    }
}


void Game::Run()
{
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // 注意 HandleInput 应该在 IsWindowResized 检查之前或之后都可以，
        // 但如果 HandleInput 中有 ToggleFullscreen，它会导致尺寸变化。
        HandleInput(); // 处理F11等

        if (IsWindowResized() && !IsWindowMinimized())
        {
            // 这个 HandleWindowResize 主要处理用户拖动窗口或最大化/恢复操作
            // ToggleFullscreen 后的尺寸变化，我们在 HandleInput 里直接处理了核心的缩放更新
            // 但为了统一，可以让 HandleWindowResize 被更广泛地调用
            // 或者确保 ToggleFullscreen 后的 UpdateRenderTextureScaling 足够。
            // 目前的设计，ToggleFullscreen 之后手动更新 screenWidth/Height 和调用 UpdateRenderTextureScaling 是可以的。
            // 如果 HandleWindowResize 做了更多事情，也可以在 ToggleFullscreen 后调用它。
            HandleWindowResize();
        }


        if (currentState == GameState::PLAYING)
        {
            UpdateGame(deltaTime);
        }
        else if (currentState == GameState::GAME_OVER)
        {
            if (IsKeyPressed(KEY_R) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                ResetGame();
            }
        }
        DrawGame();
    }
}

void Game::HandleInput()
{
    // 全屏切换 (应该在游戏状态判断之前，以便任何时候都能切换)
    if (IsKeyPressed(KEY_F11))
    {
        // 检查窗口当前是否已经是无边框全屏状态
        // Raylib 没有直接的 IsBorderlessWindowed() 函数，
        // 但我们可以通过比较窗口尺寸和屏幕尺寸来判断，或者维护一个状态变量。

        if (IsWindowFullscreen())
        {
            // 如果是独占全屏，先退出
            ToggleFullscreen(); // 这会回到窗口模式
        }

        // 切换无边框窗口状态
        // 如果当前是普通窗口，则尝试进入无边框全屏
        // 如果已经是无边框（通过尺寸判断），则恢复
        if (GetScreenWidth() == GetMonitorWidth(GetCurrentMonitor()) &&
            GetScreenHeight() == GetMonitorHeight(GetCurrentMonitor()) &&
            (IsWindowState(FLAG_WINDOW_UNDECORATED) || !IsWindowState(FLAG_WINDOW_RESIZABLE)) // 近似判断
        )
        {
            // 当前像是无边框全屏，恢复到普通窗口
            // 1. 移除无边框标志 (如果之前设置了)
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            // 2. 允许调整大小 (如果之前禁用了)
            SetWindowState(FLAG_WINDOW_RESIZABLE);
            // 3. 设置回 InitWindow 时的尺寸或一个合理的窗口尺寸
            //    这里我们用虚拟屏幕尺寸作为恢复后的窗口尺寸
            SetWindowSize(virtualScreenWidth, virtualScreenHeight);
            // 4. 居中窗口 (可选)
            SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - virtualScreenWidth / 2,
                              GetMonitorHeight(GetCurrentMonitor()) / 2 - virtualScreenHeight / 2);
        }
        else
        {
            // 当前是普通窗口，切换到无边框全屏
            int monitor = GetCurrentMonitor();
            SetWindowState(FLAG_WINDOW_UNDECORATED); // 移除边框
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            SetWindowPosition(0, 0);
            // 注意：某些情况下，SetWindowState(FLAG_WINDOW_UNDECORATED) 后再 SetWindowSize 可能效果更好
            // 或者反过来。可以试验一下。
        }

        // 任何窗口尺寸或状态改变后，都调用 HandleWindowResize
        HandleWindowResize();
    }


    if (currentState == GameState::PLAYING)
    {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (dino) dino->Jump();
        }
    }
    // Game Over 状态下的输入处理 (R键重启) 在 Run() 循环中已经有了
}

void Game::UpdateGame(float deltaTime)
{
    dino->Update(deltaTime);
    UpdateRoadSegments(deltaTime);

    // 更新障碍物
    for (auto it = obstacles.begin(); it != obstacles.end(); /* manual increment */)
    {
        it->Update(deltaTime);
        if (it->IsOffScreen(static_cast<float>(screenWidth)))
        {
            it = obstacles.erase(it);
            score += 10;
        }
        else
        {
            ++it;
        }
    }

    // 生成新的障碍物
    obstacleSpawnTimer += deltaTime;
    if (obstacleSpawnTimer >= currentObstacleSpawnInterval)
    {
        SpawnObstacle();
        obstacleSpawnTimer = 0.0f;
        // 更新下次生成间隔，可以随分数或时间增加难度
        float difficultyFactor = 1.0f - (score / 2000.0f); // 每2000分，间隔缩短一点点，最多到0
        if (difficultyFactor < 0.5f) difficultyFactor = 0.5f; // 最小缩短到一半
        float actualMinInterval = minObstacleSpawnInterval * difficultyFactor;
        float actualMaxInterval = maxObstacleSpawnInterval * difficultyFactor;
        if (actualMinInterval < 0.5f) actualMinInterval = 0.5f; // 绝对最小间隔
        if (actualMaxInterval < actualMinInterval + 0.2f) actualMaxInterval = actualMinInterval + 0.2f;


        currentObstacleSpawnInterval = actualMinInterval +
            static_cast<float>(rand()) / (static_cast<float>(
                RAND_MAX / (actualMaxInterval - actualMinInterval)));
    }

    // 游戏速度随分数增加 (可选)
    // gameSpeed = 300.0f + score / 10.0f; // 每10分速度增加1
    // if (gameSpeed > 800.0f) gameSpeed = 800.0f; // 最大速度

    CheckCollisions();
}

void Game::SpawnObstacle()
{
    if (smallCactusTextures.empty() && bigCactusTextures.empty())
    {
        TraceLog(LOG_WARNING, "No cactus textures loaded, cannot spawn obstacle.");
        return;
    }

    Texture2D chosenCactusTex;
    int typeChoice = rand() % 2; // 0 for small, 1 for big (if available)

    if (typeChoice == 0 && !smallCactusTextures.empty())
    {
        int randIdx = rand() % smallCactusTextures.size();
        chosenCactusTex = smallCactusTextures[randIdx];
    }
    else if (!bigCactusTextures.empty())
    {
        // typeChoice == 1 or small ones are unavailable
        int randIdx = rand() % bigCactusTextures.size();
        chosenCactusTex = bigCactusTextures[randIdx];
    }
    else if (!smallCactusTextures.empty())
    {
        // Fallback to small if big was chosen but unavailable
        int randIdx = rand() % smallCactusTextures.size();
        chosenCactusTex = smallCactusTextures[randIdx];
    }
    else
    {
        return; // Should not happen if check at start is done
    }

    if (chosenCactusTex.id > 0)
    {
        obstacles.emplace_back(static_cast<float>(virtualScreenWidth + 50), groundY, gameSpeed, chosenCactusTex);
    }
}

void Game::UpdateRenderTextureScaling()
{
    // 计算缩放比例，保持宽高比
    float scale = std::min(static_cast<float>(GetScreenWidth()) / virtualScreenWidth,
                           static_cast<float>(GetScreenHeight()) / virtualScreenHeight);

    // 计算渲染纹理在屏幕上绘制的区域 (destRec) 和原点，使其居中
    destRec.width = virtualScreenWidth * scale;
    destRec.height = virtualScreenHeight * scale;
    destRec.x = (static_cast<float>(GetScreenWidth()) - destRec.width) / 2.0f;
    destRec.y = (static_cast<float>(GetScreenHeight()) - destRec.height) / 2.0f;

    // 从渲染纹理中取样的区域 (sourceRec) 是整个渲染纹理
    // 但要注意 Raylib 中 RenderTexture 的 Y 轴是反的，如果直接绘制
    // sourceRec = { 0.0f, 0.0f, static_cast<float>(targetRenderTexture.texture.width), static_cast<float>(targetRenderTexture.texture.height) };
    // 为了正确显示，Y 轴需要翻转
    sourceRec = {
        0.0f, 0.0f, static_cast<float>(targetRenderTexture.texture.width),
        -static_cast<float>(targetRenderTexture.texture.height)
    };


    origin = {0.0f, 0.0f}; // 绘制时的原点
}

void Game::CheckCollisions()
{
    Rectangle dinoRect = dino->GetCollisionRect();
    for (const auto& obs : obstacles)
    {
        if (CheckCollisionRecs(dinoRect, obs.GetCollisionRect()))
        {
            currentState = GameState::GAME_OVER;
            // TODO: Play game over sound
            break;
        }
    }
}

void Game::DrawRoadSegments()
{
    if (roadSegmentTextures.empty())
    {
        // 如果没有地面纹理，画一条线
        DrawLine(0, static_cast<int>(groundY), virtualScreenWidth, static_cast<int>(groundY), DARKGRAY);
        return;
    }
    for (const auto& segment : activeRoadSegments)
    {
        DrawTexture(segment.texture, static_cast<int>(segment.xPosition),
                    static_cast<int>(groundY - segment.texture.height), WHITE);
        // 假设 groundY 是恐龙脚踩的线，地面纹理画在它的上面，底部对齐 groundY
        // 如果 groundY 指的是地面纹理的顶部Y，则 DrawTexture(..., groundY, ...)
        // 根据你的视觉需求调整：
        // 如果希望地面在恐龙脚下，则地面纹理的底部应该和 groundY 对齐。
        // DrawTexture(segment.texture, static_cast<int>(segment.xPosition), static_cast<int>(groundY - segment.texture.height), WHITE);
        // 或者，如果 groundY 就是地面图片的绘制起始 Y (顶部)
        // DrawTexture(segment.texture, static_cast<int>(segment.xPosition), static_cast<int>(groundY), WHITE);
        // 我这里假设 groundY 是恐龙脚踩的线，所以 road 的 top Y 是 groundY - road.height
        // 为了让路面看起来在恐龙脚下，并且恐龙确实踩在 groundY 上
        // 我们的 Dinosaur 和 Obstacle 的 Y 坐标是其底部与 groundY 对齐
        // 所以，如果 roadSegmentTextures 的高度是地面厚度，则其绘制Y应为 groundY
        // 即 DrawTexture(..., groundY, WHITE) 意味着 road 的顶部在 groundY
        // 这会导致路面看起来像是悬浮的，恐龙踩在路面上。
        // 我们让 groundY 代表恐龙脚踩的线，也代表地面贴图的“表面”。
        // 所以地面贴图应该画在 groundY 处，并延伸到 groundY + road_texture.height
        DrawTexture(segment.texture, static_cast<int>(segment.xPosition), static_cast<int>(groundY), WHITE);
    }
}


void Game::DrawGame()
{
    // 1. 开始绘制到渲染纹理 (我们的虚拟屏幕)
    BeginTextureMode(targetRenderTexture);
    ClearBackground(RAYWHITE); // 清除渲染纹理的背景

    // --- 所有游戏元素的绘制都发生在这里，使用虚拟坐标 ---
    DrawRoadSegments(); // 路面绘制逻辑中 X 坐标基于 virtualScreenWidth
    if (dino) dino->Draw();
    for (auto& obs : obstacles) obs.Draw();

    // 分数和 Game Over 文本也绘制在虚拟屏幕上
    DrawText(TextFormat("Score: %05d", score), 20, 20, 30, DARKGRAY);
    if (currentState == GameState::GAME_OVER)
    {
        DrawText("GAME OVER", virtualScreenWidth / 2 - MeasureText("GAME OVER", 60) / 2, virtualScreenHeight / 2 - 60,
                 60, RED);
        DrawText("Press R or Click to Restart",
                 virtualScreenWidth / 2 - MeasureText("Press R or Click to Restart", 20) / 2,
                 virtualScreenHeight / 2 + 10, 20, DARKGRAY);
    }
    // --- 结束虚拟屏幕绘制 ---

    EndTextureMode(); // 结束绘制到渲染纹理

    // 2. 开始绘制到实际窗口屏幕
    BeginDrawing();
    ClearBackground(BLACK); // 清除物理屏幕的背景 (letterbox/pillarbox 颜色)

    // 将渲染纹理绘制到屏幕上，应用缩放和居中
    DrawTexturePro(targetRenderTexture.texture, sourceRec, destRec, origin, 0.0f, WHITE);

    // (可选) 可以在这里绘制一些不参与游戏逻辑缩放的UI，比如调试信息
    DrawFPS(10, GetScreenHeight() - 30); // 在物理屏幕的左下角绘制FPS

    EndDrawing();
}

void Game::ResetGame()
{
    InitGame(); // 重新初始化游戏状态
    HandleWindowResize(); // 确保所有内容适应当前窗口大小
}

void Game::HandleWindowResize()
{
    // screenWidth 和 screenHeight 已经是物理窗口的当前尺寸
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    UpdateRenderTextureScaling(); // 更新缩放参数

    // 游戏逻辑中的 groundY 等仍然基于 virtualScreenHeight
    groundY = static_cast<float>(virtualScreenHeight) * 0.85f;

    if (dino)
    {
        dino->groundY = groundY;
        dino->position.y = groundY - dino->GetHeight();
        dino->UpdateCollisionRect();
    }

    for (auto& obs : obstacles)
    {
        if (obs.texture.id > 0)
        {
            obs.position.y = groundY - static_cast<float>(obs.texture.height);
        }
        obs.UpdateCollisionRect();
    }

    InitRoadSegments(); // 路面段基于 virtualScreenWidth
    TraceLog(LOG_INFO, "Window resized: %dx%d. Render target scale: %.2f", screenWidth, screenHeight,
             destRec.width / virtualScreenWidth);
}
