#include "../include/Game.h" // 确保此路径正确
#include "raymath.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

Game::Game(int width, int height, const char* title)
    : screenWidth(width), screenHeight(height),
      isFakeFullscreen(false),
      windowedPosX(0), windowedPosY(0),
      windowedWidth(width), windowedHeight(height),
      dino(nullptr), currentState(GameState::PLAYING),
      groundY(0),
      worldBaseScrollSpeed(400.0f),
      currentWorldScrollSpeed(worldBaseScrollSpeed),
      worldSpeedIncreaseRate(10.0f),
      timePlayed(0.0f),
      score(0),
      obstacleSpawnTimer(0.0f),
      minObstacleSpawnInterval(1.2f),
      maxObstacleSpawnInterval(2.4f),
      currentObstacleSpawnInterval(0.0f),
      jumpSound{0}, bgmMusic{0} // 初始化音频句柄
{
    // 初始化音频句柄

    srand(static_cast<unsigned int>(time(nullptr)));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, title);
    InitAudioDevice(); // 初始化音频设备
    Vector2 initialPos = GetWindowPosition();
    windowedPosX = static_cast<int>(initialPos.x);
    windowedPosY = static_cast<int>(initialPos.y);

    SetWindowMinSize(virtualScreenWidth / 2, virtualScreenHeight / 2);
    SetTargetFPS(160);

    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_POINT);

    LoadResources();
    InitGame();
    HandleWindowResize();
}

Game::~Game()
{
    UnloadRenderTexture(targetRenderTexture);
    UnloadResources();
    delete dino;
    CloseAudioDevice(); // 关闭音频设备
    CloseWindow();
}

void Game::LoadResources()
{
    Texture2D tempTex;
    // 清空容器
    dinoRunFrames.clear();
    dinoSneakFrames.clear();
    smallCactusTextures.clear();
    bigCactusTextures.clear();
    roadSegmentTextures.clear();
    birdFrames.clear();

    // Dino Run (确保文件名是 _1, _2)
    tempTex = LoadTexture("assets/images/dino_run_1.png"); // 修正路径
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        dinoRunFrames.push_back(tempTex);
    }
    else { TraceLog(LOG_WARNING, "Failed: assets/images/dino_run_1.png"); }
    tempTex = LoadTexture("assets/images/dino_run_2.png"); // 修正路径
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        dinoRunFrames.push_back(tempTex);
    }
    else { TraceLog(LOG_WARNING, "Failed: assets/images/dino_run_2.png"); }

    // Dino Sneak (确保文件名是 _1, _2)
    tempTex = LoadTexture("assets/images/dino_sneak_1.png"); // 修正路径
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        dinoSneakFrames.push_back(tempTex);
    }
    else { TraceLog(LOG_WARNING, "Failed: assets/images/dino_sneak_1.png"); }
    tempTex = LoadTexture("assets/images/dino_sneak_2.png"); // 修正路径
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        dinoSneakFrames.push_back(tempTex);
    }
    else { TraceLog(LOG_WARNING, "Failed: assets/images/dino_sneak_2.png"); }

    // Cacti, Road, Birds (确保路径是 "assets/images/")
    tempTex = LoadTexture("assets/images/small_cactus_1.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        smallCactusTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/small_cactus_1.png");
    tempTex = LoadTexture("assets/images/small_cactus_2.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        smallCactusTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/small_cactus_2.png");
    tempTex = LoadTexture("assets/images/small_cactus_3.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        smallCactusTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/small_cactus_3.png");

    tempTex = LoadTexture("assets/images/big_cactus_1.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        bigCactusTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/big_cactus_1.png");
    tempTex = LoadTexture("assets/images/big_cactus_2.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        bigCactusTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/big_cactus_2.png");

    tempTex = LoadTexture("assets/images/road_1.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        roadSegmentTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/road_1.png");
    tempTex = LoadTexture("assets/images/road_2.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        roadSegmentTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/road_2.png");
    tempTex = LoadTexture("assets/images/road_3.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        roadSegmentTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/road_3.png");
    tempTex = LoadTexture("assets/images/road_4.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        roadSegmentTextures.push_back(tempTex);
    }
    else TraceLog(LOG_WARNING, "Failed: assets/images/road_4.png");

    tempTex = LoadTexture("assets/images/bird_1.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        birdFrames.push_back(tempTex);
    }
    else { TraceLog(LOG_WARNING, "Failed: assets/images/bird_1.png"); }
    tempTex = LoadTexture("assets/images/bird_2.png");
    if (tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        birdFrames.push_back(tempTex);
    }
    else { TraceLog(LOG_WARNING, "Failed: assets/images/bird_2.png"); }

    // 加载音效和音乐
    jumpSound = LoadSound("assets/sounds/jump.wav"); // 修正路径
    if (jumpSound.frameCount == 0) TraceLog(LOG_WARNING, "Failed to load sound: assets/sounds/jump.wav");

    bgmMusic = LoadMusicStream("assets/sounds/bgm.wav"); // 修正路径
    if (bgmMusic.frameCount == 0)
    {
        TraceLog(LOG_WARNING, "Failed to load music stream: assets/sounds/bgm.wav");
    }
    else
    {
        SetMusicVolume(bgmMusic, 0.3f); // 设置BGM音量
    }

    if (dinoRunFrames.empty())
    {
        TraceLog(LOG_ERROR, "CRITICAL: Dinosaur has no run frames for animation.");
    }
}

void Game::UnloadResources()
{
    for (auto& tex : dinoRunFrames) if (tex.id > 0) UnloadTexture(tex);
    dinoRunFrames.clear();
    for (auto& tex : dinoSneakFrames) if (tex.id > 0) UnloadTexture(tex);
    dinoSneakFrames.clear();
    for (auto& tex : smallCactusTextures) if (tex.id > 0) UnloadTexture(tex);
    smallCactusTextures.clear();
    for (auto& tex : bigCactusTextures) if (tex.id > 0) UnloadTexture(tex);
    bigCactusTextures.clear();
    for (auto& tex : roadSegmentTextures) if (tex.id > 0) UnloadTexture(tex);
    roadSegmentTextures.clear();
    for (auto& tex : birdFrames) if (tex.id > 0) UnloadTexture(tex);
    birdFrames.clear();

    if (jumpSound.frameCount > 0) UnloadSound(jumpSound);
    if (bgmMusic.frameCount > 0)
    {
        StopMusicStream(bgmMusic);
        UnloadMusicStream(bgmMusic);
    }
}

void Game::InitGame()
{
    groundY = static_cast<float>(virtualScreenHeight) * 0.85f;
    if (dino) delete dino;
    // *** 创建 Dinosaur 时传递 jumpSound ***
    dino = new Dinosaur(virtualScreenWidth / 4.0f, groundY,
                        dinoRunFrames, dinoSneakFrames, jumpSound); // <--- 传递声音

    obstacles.clear();
    birds.clear();
    score = 0;
    timePlayed = 0.0f;
    worldBaseScrollSpeed = 200.0f;
    currentWorldScrollSpeed = worldBaseScrollSpeed;
    obstacleSpawnTimer = 0.0f;
    currentObstacleSpawnInterval = minObstacleSpawnInterval + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX
        / (maxObstacleSpawnInterval - minObstacleSpawnInterval)));

    InitRoadSegments();
    currentState = GameState::PLAYING;

    if (bgmMusic.frameCount > 0 && IsAudioDeviceReady())
    {
        SeekMusicStream(bgmMusic, 0.0f);
        PlayMusicStream(bgmMusic);
    }
}

void Game::HandleInput()
{
    if (IsKeyPressed(KEY_F11))
    {
        int currentMonitor = GetCurrentMonitor();
        if (IsWindowFullscreen())
        {
            ToggleFullscreen();
            isFakeFullscreen = false;
        }

        if (!isFakeFullscreen)
        {
            if (!IsWindowMaximized())
            {
                windowedWidth = screenWidth;
                windowedHeight = screenHeight;
                Vector2 pos = GetWindowPosition();
                windowedPosX = static_cast<int>(pos.x);
                windowedPosY = static_cast<int>(pos.y);
            }
            else
            {
                windowedWidth = virtualScreenWidth;
                windowedHeight = virtualScreenHeight;
                windowedPosX = GetMonitorWidth(currentMonitor) / 2 - windowedWidth / 2;
                windowedPosY = GetMonitorHeight(currentMonitor) / 2 - windowedHeight / 2;
            }
            SetWindowState(FLAG_WINDOW_UNDECORATED);
            SetWindowSize(GetMonitorWidth(currentMonitor), GetMonitorHeight(currentMonitor));
            SetWindowPosition(0, 0);
            isFakeFullscreen = true;
        }
        else
        {
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            SetWindowSize(windowedWidth, windowedHeight);
            SetWindowPosition(windowedPosX, windowedPosY);
            SetWindowState(FLAG_WINDOW_RESIZABLE);
            isFakeFullscreen = false;
        }
        HandleWindowResize();
    }

    if (currentState == GameState::PLAYING && dino)
    {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            // *** 不再在这里播放声音 ***
            // if (dino->IsOnGround() || dino->coyoteTimeCounter > 0.0f) {
            //      if (jumpSound.frameCount > 0 && IsAudioDeviceReady()) PlaySound(jumpSound);
            // }
            dino->RequestJump(); // 只请求跳跃
        }
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        {
            dino->StartSneaking();
        }
        else
        {
            dino->StopSneaking();
        }
        float moveDirection = 0.0f;
        if (IsKeyDown(KEY_D)) moveDirection += 1.0f;
        if (IsKeyDown(KEY_A)) moveDirection -= 1.0f;
        dino->Move(moveDirection, GetFrameTime());
    }
}

void Game::UpdateGame(float deltaTime)
{
    if (!dino || currentState == GameState::GAME_OVER) return;

    timePlayed += deltaTime;
    score = static_cast<int>(timePlayed * 10);

    worldBaseScrollSpeed += worldSpeedIncreaseRate * deltaTime;
    worldBaseScrollSpeed = std::min(worldBaseScrollSpeed, 750.0f);
    currentWorldScrollSpeed = worldBaseScrollSpeed;

    dino->Update(deltaTime);

    if (dino->position.x < 0) dino->position.x = 0;
    if (dino->position.x + dino->GetWidth() > virtualScreenWidth)
    {
        dino->position.x = virtualScreenWidth - dino->GetWidth();
    }

    UpdateRoadSegments(deltaTime);

    for (auto it = obstacles.begin(); it != obstacles.end();)
    {
        it->speed = currentWorldScrollSpeed;
        it->Update(deltaTime);
        if (it->IsOffScreen(static_cast<float>(virtualScreenWidth))) it = obstacles.erase(it);
        else ++it;
    }
    for (auto it = birds.begin(); it != birds.end();)
    {
        it->speed = currentWorldScrollSpeed * 0.92f;
        it->Update(deltaTime);
        if (it->IsOffScreen()) it = birds.erase(it);
        else ++it;
    }

    obstacleSpawnTimer += deltaTime;
    if (obstacleSpawnTimer >= currentObstacleSpawnInterval)
    {
        SpawnObstacleOrBird();
        obstacleSpawnTimer = 0.0f;
        float difficultyFactor = 1.0f - (timePlayed / 200.0f);
        difficultyFactor = std::max(0.20f, difficultyFactor);
        float actualMin = minObstacleSpawnInterval * difficultyFactor;
        float actualMax = maxObstacleSpawnInterval * difficultyFactor;
        if (actualMin < 0.35f) actualMin = 0.35f;
        if (actualMax < actualMin + 0.18f) actualMax = actualMin + 0.18f;
        currentObstacleSpawnInterval = actualMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (
            actualMax - actualMin)));
    }
    CheckCollisions();
}

void Game::SpawnObstacleOrBird()
{
    if (birdFrames.empty() && smallCactusTextures.empty() && bigCactusTextures.empty()) return;
    float spawnX = static_cast<float>(virtualScreenWidth) + 250.0f + (rand() % 350);

    int entityTypeRoll = rand() % 100;
    if (entityTypeRoll < 80 || birdFrames.empty())
    {
        // 90% 仙人掌
        Texture2D chosenCactusTex;
        bool preferSmall = (rand() % 3 != 0 && !smallCactusTextures.empty()) || bigCactusTextures.empty();
        if (preferSmall && !smallCactusTextures.empty())
            chosenCactusTex = smallCactusTextures[rand() %
                smallCactusTextures.size()];
        else if (!bigCactusTextures.empty()) chosenCactusTex = bigCactusTextures[rand() % bigCactusTextures.size()];
        else if (!smallCactusTextures.empty())
            chosenCactusTex = smallCactusTextures[rand() % smallCactusTextures.
                size()];
        else return;
        if (chosenCactusTex.id > 0) obstacles.emplace_back(spawnX, groundY, currentWorldScrollSpeed, chosenCactusTex);
    }
    else
    {
        // 10% 鸟
        if (!birdFrames.empty() && dino)
        {
            float dinoStandingTop = groundY - dino->runHeight; // 使用 runHeight
            float birdHeight = birdFrames[0].height;
            float spawnY;
            int heightTier = rand() % 2;
            if (heightTier == 0) spawnY = dinoStandingTop - 40 - birdHeight - (rand() % 70);
            else spawnY = groundY - dino->sneakHeight - birdHeight - 10 - (rand() % 20); // 使用 sneakHeight 计算低飞高度
            spawnY = std::max(virtualScreenHeight * 0.15f, std::min(spawnY, groundY - birdHeight - 25.0f));
            birds.emplace_back(spawnX, spawnY, currentWorldScrollSpeed * 0.92f, birdFrames);
        }
    }
}

void Game::CheckCollisions()
{
    if (!dino || currentState == GameState::GAME_OVER) return;
    Rectangle dinoRect = dino->GetCollisionRect(); // 使用调整后的碰撞盒
    for (const auto& obs : obstacles)
    {
        if (CheckCollisionRecs(dinoRect, obs.GetCollisionRect()))
        {
            currentState = GameState::GAME_OVER;
            return;
        }
    }
    for (const auto& brd : birds)
    {
        if (CheckCollisionRecs(dinoRect, brd.GetCollisionRect()))
        {
            currentState = GameState::GAME_OVER;
            return;
        }
    }
}

void Game::DrawGame()
{
    BeginTextureMode(targetRenderTexture);
    ClearBackground(RAYWHITE);
    DrawRoadSegments();
    if (dino) dino->Draw();
    for (auto& obs : obstacles) obs.Draw();
    for (auto& brd : birds) brd.Draw();
    DrawText(TextFormat("Score: %06d", score), 20, 20, 30, DARKGRAY);
    DrawText(TextFormat("Time: %.1fs", timePlayed),
             virtualScreenWidth - MeasureText(TextFormat("Time: %.1fs", timePlayed), 20) - 20, 20, 20, DARKGRAY);
    if (currentState == GameState::GAME_OVER)
    {
        DrawText("GAME OVER", virtualScreenWidth / 2 - MeasureText("GAME OVER", 70) / 2, virtualScreenHeight * 0.4f, 70,
                 RED);
        DrawText("Press R or Click to Restart",
                 virtualScreenWidth / 2 - MeasureText("Press R or Click to Restart", 25) / 2,
                 virtualScreenHeight * 0.6f, 25, DARKGRAY);
    }
    EndTextureMode();
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(targetRenderTexture.texture, sourceRec, destRec, origin, 0.0f, WHITE);
    EndDrawing();
}

void Game::ResetGame()
{
    if (bgmMusic.frameCount > 0 && IsMusicStreamPlaying(bgmMusic))
    {
        StopMusicStream(bgmMusic);
        // SeekMusicStream(bgmMusic, 0.0f); // InitGame 会处理播放和 Seek
    }
    InitGame();
    HandleWindowResize();
}

void Game::HandleWindowResize()
{
    if (isFakeFullscreen &&
        (GetScreenWidth() != GetMonitorWidth(GetCurrentMonitor()) ||
            GetScreenHeight() != GetMonitorHeight(GetCurrentMonitor())))
    {
        isFakeFullscreen = false;
    }
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    UpdateRenderTextureScaling();
    groundY = static_cast<float>(virtualScreenHeight) * 0.85f;
    if (dino)
    {
        dino->groundY = groundY;
        dino->position.y = groundY - dino->GetHeight();
        dino->UpdateCollisionRect();
    }
    InitRoadSegments(); // 窗口大小改变时重新初始化路面可能更简单
    TraceLog(LOG_INFO, "Window resized/state changed: Phys: %dx%d, Virt: %dx%d. Render scale: %.2f. FakeFullscreen: %s",
             screenWidth, screenHeight, virtualScreenWidth, virtualScreenHeight, destRec.width / virtualScreenWidth,
             isFakeFullscreen ? "ON" : "OFF");
}

void Game::UpdateRenderTextureScaling()
{
    float scale = std::min(static_cast<float>(screenWidth) / virtualScreenWidth,
                           static_cast<float>(screenHeight) / virtualScreenHeight);
    destRec.width = virtualScreenWidth * scale;
    destRec.height = virtualScreenHeight * scale;
    destRec.x = (static_cast<float>(screenWidth) - destRec.width) / 2.0f;
    destRec.y = (static_cast<float>(screenHeight) - destRec.height) / 2.0f;
    sourceRec = {
        0.0f, 0.0f, static_cast<float>(targetRenderTexture.texture.width),
        -static_cast<float>(targetRenderTexture.texture.height)
    };
    origin = {0.0f, 0.0f};
}

void Game::InitRoadSegments()
{
    activeRoadSegments.clear();
    if (roadSegmentTextures.empty() || roadSegmentTextures[0].id == 0) return;
    float currentX = 0.0f;
    while (currentX < virtualScreenWidth * 1.5f)
    {
        int randIdx = rand() % roadSegmentTextures.size();
        Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
        activeRoadSegments.push_back({chosenRoadTex, currentX});
        currentX += chosenRoadTex.width;
    }
}

void Game::UpdateRoadSegments(float deltaTime)
{
    if (roadSegmentTextures.empty()) return;
    for (auto& segment : activeRoadSegments)
    {
        segment.xPosition -= currentWorldScrollSpeed * deltaTime;
    }
    while (!activeRoadSegments.empty() && (activeRoadSegments.front().xPosition + activeRoadSegments.front().texture.
        width) < 0)
    {
        activeRoadSegments.pop_front();
    }
    float rightmostX = 0.0f;
    if (!activeRoadSegments.empty())
    {
        rightmostX = activeRoadSegments.back().xPosition + activeRoadSegments.back().texture.width;
    }
    else
    {
        InitRoadSegments();
        if (!activeRoadSegments.empty())
        {
            rightmostX = activeRoadSegments.back().xPosition + activeRoadSegments.back().texture.width;
        }
        else return;
    }
    while (rightmostX < virtualScreenWidth * 1.5f)
    {
        int randIdx = rand() % roadSegmentTextures.size();
        Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
        activeRoadSegments.push_back({chosenRoadTex, rightmostX});
        rightmostX += chosenRoadTex.width;
    }
}

void Game::DrawRoadSegments()
{
    if (roadSegmentTextures.empty())
    {
        DrawLine(0, static_cast<int>(groundY), virtualScreenWidth, static_cast<int>(groundY), DARKGRAY);
        return;
    }
    for (const auto& segment : activeRoadSegments)
    {
        DrawTexture(segment.texture, static_cast<int>(segment.xPosition), static_cast<int>(groundY), WHITE);
    }
}

void Game::Run()
{
    while (!WindowShouldClose())
    {
        // --- BGM Update & Loop ---
        if (bgmMusic.frameCount > 0 && IsAudioDeviceReady())
        {
            if (!IsMusicStreamPlaying(bgmMusic) && currentState == GameState::PLAYING)
            {
                PlayMusicStream(bgmMusic);
            }
            if (IsMusicStreamPlaying(bgmMusic))
            {
                UpdateMusicStream(bgmMusic);
                if (GetMusicTimePlayed(bgmMusic) >= GetMusicTimeLength(bgmMusic) - 0.1f)
                {
                    // 手动循环
                    SeekMusicStream(bgmMusic, 0.0f);
                }
            }
        }

        float deltaTime = GetFrameTime();
        HandleInput();
        if (IsWindowResized() && !IsWindowMinimized())
        {
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
