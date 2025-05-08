#include "../include/Game.h"
#include "raymath.h"
#include <iostream>
#include <cstdlib> // For rand(), srand()
#include <ctime>   // For time()

Game::Game(int width, int height, const char *title)
    : screenWidth(width), screenHeight(height),
      dino(nullptr), currentState(GameState::PLAYING),
      groundY(0), gameSpeed(300.0f), score(0),
      obstacleSpawnTimer(0.0f),
      minObstacleSpawnInterval(1.0f), maxObstacleSpawnInterval(2.5f),
      currentObstacleSpawnInterval(0.0f), // Will be set in InitGame/ResetGame
      totalRoadWidthCovered(0.0f) {
    srand(static_cast<unsigned int>(time(nullptr))); // 初始化随机种子

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, title);
    SetWindowMinSize(600, 300); // 设置最小窗口尺寸
    SetTargetFPS(60);

    LoadResources();
    InitGame(); // 初始化游戏状态和对象
    // 确保 HandleWindowResize 在一开始就被调用以正确设置所有依赖屏幕尺寸的变量
    HandleWindowResize();
}

Game::~Game() {
    UnloadResources();
    delete dino;
    CloseWindow();
}

void Game::LoadResources() {
    // Dino
    dinoRunFrames.push_back(LoadTexture("assets/dino_run_0.png"));
    dinoRunFrames.push_back(LoadTexture("assets/dino_run_1.png"));
    for (const auto &tex: dinoRunFrames) {
        if (tex.id == 0) TraceLog(LOG_WARNING, "Failed to load a dino run frame.");
    }

    dinoJumpFrame = LoadTexture("assets/dino_jump.png");
    if (dinoJumpFrame.id == 0) TraceLog(LOG_WARNING, "Failed to load dino jump frame.");

    // Small Cacti
    smallCactusTextures.push_back(LoadTexture("assets/small_cactus_1.png"));
    smallCactusTextures.push_back(LoadTexture("assets/small_cactus_2.png"));
    smallCactusTextures.push_back(LoadTexture("assets/small_cactus_3.png"));
    for (const auto &tex: smallCactusTextures) {
        if (tex.id == 0) TraceLog(LOG_WARNING, "Failed to load a small cactus texture.");
    }

    // Big Cacti
    bigCactusTextures.push_back(LoadTexture("assets/big_cactus_1.png"));
    bigCactusTextures.push_back(LoadTexture("assets/big_cactus_2.png"));
    // big_cactus_3.png 似乎不存在于你的列表，如果需要请添加
    for (const auto &tex: bigCactusTextures) {
        if (tex.id == 0) TraceLog(LOG_WARNING, "Failed to load a big cactus texture.");
    }

    // Road Segments
    roadSegmentTextures.push_back(LoadTexture("assets/road_1.png"));
    roadSegmentTextures.push_back(LoadTexture("assets/road_2.png"));
    roadSegmentTextures.push_back(LoadTexture("assets/road_3.png"));
    roadSegmentTextures.push_back(LoadTexture("assets/road_4.png"));
    for (const auto &tex: roadSegmentTextures) {
        if (tex.id == 0) TraceLog(LOG_WARNING, "Failed to load a road segment texture.");
    }
}

void Game::UnloadResources() {
    for (auto &tex: dinoRunFrames) UnloadTexture(tex);
    for (auto &tex: smallCactusTextures) UnloadTexture(tex);
    for (auto &tex: bigCactusTextures) UnloadTexture(tex);
    for (auto &tex: roadSegmentTextures) UnloadTexture(tex);
    UnloadTexture(dinoJumpFrame);
    dinoRunFrames.clear();
    smallCactusTextures.clear();
    bigCactusTextures.clear();
    roadSegmentTextures.clear();
}

void Game::InitGame() {
    // groundY 会在 HandleWindowResize 中设置，或基于初始 screenHeight
    // 确保在创建恐龙之前 groundY 是正确的
    this->groundY = static_cast<float>(screenHeight) * 0.85f; // 地面在屏幕85%的高度

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


void Game::InitRoadSegments() {
    activeRoadSegments.clear();
    totalRoadWidthCovered = 0.0f;

    if (roadSegmentTextures.empty() || roadSegmentTextures[0].id == 0) {
        TraceLog(LOG_WARNING, "Road textures not loaded, cannot initialize road segments.");
        return;
    }

    // 初始填充路面，直到覆盖屏幕宽度再加一个路段的宽度作为缓冲
    while (totalRoadWidthCovered < screenWidth + roadSegmentTextures[0].width) {
        // 假设所有路段宽度相似
        int randIdx = rand() % roadSegmentTextures.size();
        Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
        activeRoadSegments.push_back({chosenRoadTex, totalRoadWidthCovered});
        totalRoadWidthCovered += chosenRoadTex.width;
    }
}

void Game::UpdateRoadSegments(float deltaTime) {
    if (roadSegmentTextures.empty()) return;

    totalRoadWidthCovered = 0; // 重新计算当前覆盖宽度
    float rightmostX = 0;

    for (auto &segment: activeRoadSegments) {
        segment.xPosition -= gameSpeed * deltaTime;
        if (segment.xPosition + segment.texture.width > rightmostX) {
            rightmostX = segment.xPosition + segment.texture.width;
        }
    }
    totalRoadWidthCovered = rightmostX; // 最右边的路段的右边缘

    // 移除完全移出屏幕左侧的路段
    while (!activeRoadSegments.empty() && activeRoadSegments.front().xPosition + activeRoadSegments.front().texture.
           width < 0) {
        activeRoadSegments.pop_front();
    }

    // 如果右侧有空间，添加新的路段
    while (totalRoadWidthCovered < screenWidth + roadSegmentTextures[0].width * 2) {
        // 保留一些缓冲
        int randIdx = rand() % roadSegmentTextures.size();
        Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
        float newX = totalRoadWidthCovered; // 新路段紧接着之前的路段
        if (!activeRoadSegments.empty()) {
            // 以免totalRoadWidthCovered在移除后未及时更新
            newX = activeRoadSegments.back().xPosition + activeRoadSegments.back().texture.width;
        } else {
            // 如果路段都移除了，从0开始
            newX = 0;
        }

        activeRoadSegments.push_back({chosenRoadTex, newX});
        totalRoadWidthCovered = newX + chosenRoadTex.width;
    }
}


void Game::Run() {
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsWindowResized() && !IsWindowMinimized()) {
            // 窗口最小化时不处理resize
            HandleWindowResize();
        }

        HandleInput();

        if (currentState == GameState::PLAYING) {
            UpdateGame(deltaTime);
        } else if (currentState == GameState::GAME_OVER) {
            if (IsKeyPressed(KEY_R) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                ResetGame();
            }
        }
        DrawGame();
    }
}

void Game::HandleInput() {
    if (currentState == GameState::PLAYING) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            dino->Jump();
        }
    }
}

void Game::UpdateGame(float deltaTime) {
    dino->Update(deltaTime);
    UpdateRoadSegments(deltaTime);

    // 更新障碍物
    for (auto it = obstacles.begin(); it != obstacles.end(); /* manual increment */) {
        it->Update(deltaTime);
        if (it->IsOffScreen(static_cast<float>(screenWidth))) {
            it = obstacles.erase(it);
            score += 10;
        } else {
            ++it;
        }
    }

    // 生成新的障碍物
    obstacleSpawnTimer += deltaTime;
    if (obstacleSpawnTimer >= currentObstacleSpawnInterval) {
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

void Game::SpawnObstacle() {
    if (smallCactusTextures.empty() && bigCactusTextures.empty()) {
        TraceLog(LOG_WARNING, "No cactus textures loaded, cannot spawn obstacle.");
        return;
    }

    Texture2D chosenCactusTex;
    int typeChoice = rand() % 2; // 0 for small, 1 for big (if available)

    if (typeChoice == 0 && !smallCactusTextures.empty()) {
        int randIdx = rand() % smallCactusTextures.size();
        chosenCactusTex = smallCactusTextures[randIdx];
    } else if (!bigCactusTextures.empty()) {
        // typeChoice == 1 or small ones are unavailable
        int randIdx = rand() % bigCactusTextures.size();
        chosenCactusTex = bigCactusTextures[randIdx];
    } else if (!smallCactusTextures.empty()) {
        // Fallback to small if big was chosen but unavailable
        int randIdx = rand() % smallCactusTextures.size();
        chosenCactusTex = smallCactusTextures[randIdx];
    } else {
        return; // Should not happen if check at start is done
    }

    if (chosenCactusTex.id > 0) {
        obstacles.emplace_back(static_cast<float>(screenWidth + 50), groundY, gameSpeed, chosenCactusTex);
    }
}

void Game::CheckCollisions() {
    Rectangle dinoRect = dino->GetCollisionRect();
    for (const auto &obs: obstacles) {
        if (CheckCollisionRecs(dinoRect, obs.GetCollisionRect())) {
            currentState = GameState::GAME_OVER;
            // TODO: Play game over sound
            break;
        }
    }
}

void Game::DrawRoadSegments() {
    if (roadSegmentTextures.empty()) {
        // 如果没有地面纹理，画一条线
        DrawLine(0, static_cast<int>(groundY), screenWidth, static_cast<int>(groundY), DARKGRAY);
        return;
    }
    for (const auto &segment: activeRoadSegments) {
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


void Game::DrawGame() {
    BeginDrawing();
    ClearBackground(RAYWHITE); // 天空背景色

    DrawRoadSegments(); // 绘制地面

    if (dino) dino->Draw(); // 绘制恐龙

    for (auto &obs: obstacles) {
        // 绘制障碍物
        obs.Draw();
    }

    // 绘制分数
    DrawText(TextFormat("Score: %05d", score), 20, 20, 30, DARKGRAY);
    // DrawText(TextFormat("Speed: %.0f", gameSpeed), 20, 60, 20, DARKGRAY); // Debug speed

    if (currentState == GameState::GAME_OVER) {
        DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 60) / 2, screenHeight / 2 - 60, 60, RED);
        DrawText("Press R or Click to Restart", screenWidth / 2 - MeasureText("Press R or Click to Restart", 20) / 2,
                 screenHeight / 2 + 10, 20, DARKGRAY);
    }

    EndDrawing();
}

void Game::ResetGame() {
    InitGame(); // 重新初始化游戏状态
    HandleWindowResize(); // 确保所有内容适应当前窗口大小
}

void Game::HandleWindowResize() {
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    // 更新地面Y坐标
    groundY = static_cast<float>(screenHeight) * 0.85f; // 地面在屏幕85%的高度

    // 更新恐龙位置和内部 groundY
    if (dino) {
        dino->groundY = groundY;
        // 当窗口调整时，恐龙的脚应该仍然在新的 groundY 上
        dino->position.y = groundY - dino->GetHeight();
        dino->UpdateCollisionRect();
    }

    // 更新现有障碍物的位置（如果需要，但通常障碍物生成时会使用新的screenWidth和groundY）
    // 如果障碍物Y坐标依赖旧的groundY，则需要调整，但我们的实现是让其底部对齐新的groundY
    for (auto &obs: obstacles) {
        if (obs.texture.id > 0) {
            obs.position.y = groundY - static_cast<float>(obs.texture.height);
        } // else 保持，因为没有纹理高度信息
        obs.UpdateCollisionRect();
    }

    // 重新初始化/调整路面段以适应新的屏幕宽度
    InitRoadSegments(); // 最简单的方法是重新生成路面
    TraceLog(LOG_INFO, "Window resized: %dx%d, New groundY: %.2f", screenWidth, screenHeight, groundY);
}
