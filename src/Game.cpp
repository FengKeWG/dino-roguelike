#include "../include/Game.h" // 确保此路径正确
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

Game::Game(const int width, const int height, const char* title)
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
      minObstacleSpawnInterval(0.6f),
      maxObstacleSpawnInterval(2.4f),
      currentObstacleSpawnInterval(0.0f),
      jumpSound{nullptr}, bgmMusic{nullptr}
{
    srand(static_cast<unsigned int>(time(nullptr)));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, title);
    SetExitKey(KEY_NULL);
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
    // 清空容器
    dinoRunFrames.clear();
    dinoSneakFrames.clear();
    smallCactusTextures.clear();
    bigCactusTextures.clear();
    roadSegmentTextures.clear();
    birdFrames.clear();

    // Dino Run (确保文件名是 _1, _2)
    Texture2D tempTex = LoadTexture("assets/images/dino_run_1.png"); // 修正路径
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

    dashSound = LoadSound("assets/sounds/dash.wav"); // <--- 加载冲刺音效
    if (dashSound.frameCount == 0) TraceLog(LOG_WARNING, "Failed to load sound: assets/sounds/dash.wav");

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
    for (const auto& tex : dinoRunFrames) if (tex.id > 0) UnloadTexture(tex);
    dinoRunFrames.clear();
    for (const auto& tex : dinoSneakFrames) if (tex.id > 0) UnloadTexture(tex);
    dinoSneakFrames.clear();
    for (const auto& tex : smallCactusTextures) if (tex.id > 0) UnloadTexture(tex);
    smallCactusTextures.clear();
    for (const auto& tex : bigCactusTextures) if (tex.id > 0) UnloadTexture(tex);
    bigCactusTextures.clear();
    for (const auto& tex : roadSegmentTextures) if (tex.id > 0) UnloadTexture(tex);
    roadSegmentTextures.clear();
    for (const auto& tex : birdFrames) if (tex.id > 0) UnloadTexture(tex);
    birdFrames.clear();

    if (jumpSound.frameCount > 0) UnloadSound(jumpSound);
    if (dashSound.frameCount > 0) UnloadSound(dashSound);
    if (bgmMusic.frameCount > 0)
    {
        StopMusicStream(bgmMusic);
        UnloadMusicStream(bgmMusic);
    }
}

void Game::InitGame()
{
    groundY = static_cast<float>(virtualScreenHeight) * 0.85f;
    delete dino;
    // *** 创建 Dinosaur 时传递 jumpSound ***
    dino = new Dinosaur(virtualScreenWidth / 4.0f, groundY,
                        dinoRunFrames, dinoSneakFrames,
                        jumpSound, dashSound); // <--- 传递声音

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
        const int currentMonitor = GetCurrentMonitor();
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
                const auto [x, y] = GetWindowPosition();
                windowedPosX = static_cast<int>(x);
                windowedPosY = static_cast<int>(y);
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

    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (currentState == GameState::PLAYING)
        {
            currentState = GameState::PAUSED;
            if (bgmMusic.frameCount > 0 && IsMusicStreamPlaying(bgmMusic))
            {
                PauseMusicStream(bgmMusic);
            }
            TraceLog(LOG_INFO, "Game Paused");
        }
        else if (currentState == GameState::PAUSED)
        {
            currentState = GameState::PLAYING;
            if (bgmMusic.frameCount > 0)
            {
                ResumeMusicStream(bgmMusic);
            }
            TraceLog(LOG_INFO, "Game Resumed");
        }
    }
    if (currentState == GameState::PLAYING && dino)
    {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            dino->RequestJump();
        }
        if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT))
        {
            dino->RequestDash();
        }
        if (IsKeyDown(KEY_S))
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

void Game::UpdateGame(const float deltaTime)
{
    if (currentState == GameState::GAME_OVER || currentState == GameState::PAUSED)
    {
        return;
    }

    if (!dino) return;

    timePlayed += deltaTime;
    score = static_cast<int>(timePlayed * 10);

    worldBaseScrollSpeed += worldSpeedIncreaseRate * deltaTime;
    currentWorldScrollSpeed = worldBaseScrollSpeed; // 这个是正值

    // --- 调用 dino->Update 时传递世界滚动速度 ---
    dino->Update(deltaTime, currentWorldScrollSpeed); // Dinosaur内部会处理方向

    // 恐龙边界检查 (与之前相同)
    if (dino->position.x < 0) dino->position.x = 0;
    if (dino->position.x + dino->GetWidth() > virtualScreenWidth)
    {
        dino->position.x = virtualScreenWidth - dino->GetWidth();
    }

    UpdateRoadSegments(deltaTime); // 路面滚动

    // 障碍物和鸟的更新 (与之前相同)
    for (auto it = obstacles.begin(); it != obstacles.end();)
    {
        it->speed = currentWorldScrollSpeed; // 障碍物使用正的速度，在内部处理为向左移动
        it->Update(deltaTime);
        if (it->IsOffScreen(static_cast<float>(virtualScreenWidth))) it = obstacles.erase(it);
        else ++it;
    }
    for (auto it = birds.begin(); it != birds.end();)
    {
        const float birdSpeedFactor = 0.3f + (static_cast<float>(rand() % 221) / 100.0f);
        it->speed = currentWorldScrollSpeed * birdSpeedFactor;
        it->Update(deltaTime);
        if (it->IsOffScreen()) it = birds.erase(it);
        else ++it;
    }

    obstacleSpawnTimer += deltaTime;
    if (obstacleSpawnTimer >= currentObstacleSpawnInterval)
    {
        SpawnObstacleOrBird();
        obstacleSpawnTimer = 0.0f;
        currentObstacleSpawnInterval = minObstacleSpawnInterval + static_cast<float>(rand()) / (static_cast<float>(
            RAND_MAX / (maxObstacleSpawnInterval - minObstacleSpawnInterval)));
    }
    CheckCollisions();
}

void Game::SpawnObstacleOrBird()
{
    if (birdFrames.empty() && smallCactusTextures.empty() && bigCactusTextures.empty()) return;
    float spawnX = static_cast<float>(virtualScreenWidth) + 250.0f + (rand() % 350);

    if (const int entityTypeRoll = rand() % 100; entityTypeRoll < 60 || birdFrames.empty()) // 60% 仙人掌，或者没有鸟帧
    {
        Texture2D chosenCactusTex;
        if (const bool preferSmall = (rand() % 3 != 0 && !smallCactusTextures.empty()) || bigCactusTextures.empty();
            preferSmall && !smallCactusTextures.empty())
            chosenCactusTex = smallCactusTextures[rand() % smallCactusTextures.size()];
        else if (!bigCactusTextures.empty()) chosenCactusTex = bigCactusTextures[rand() % bigCactusTextures.size()];
        else if (!smallCactusTextures.empty())
            chosenCactusTex = smallCactusTextures[rand() % smallCactusTextures.size()];
        else return;
        if (chosenCactusTex.id > 0) obstacles.emplace_back(spawnX, groundY, currentWorldScrollSpeed, chosenCactusTex);
    }
    else
    {
        if (!birdFrames.empty())
        {
            const float dinoStandingTop = groundY - dino->runHeight;
            const float birdHeight = birdFrames[0].height;
            float spawnY;
            if (const int heightTier = rand() % 2; heightTier == 0)
                spawnY = dinoStandingTop - 60 - birdHeight - (rand()
                    % 80);
            else spawnY = groundY - dino->sneakHeight - birdHeight - 20 - (rand() % 30);
            spawnY = std::max(virtualScreenHeight * 0.15f, std::min(spawnY, groundY - birdHeight - 25.0f));
            birds.emplace_back(spawnX, spawnY, currentWorldScrollSpeed, birdFrames);
        }
    }
}

void Game::CheckCollisions()
{
    const Rectangle dinoRect = dino->GetCollisionRect(); // 使用调整后的碰撞盒
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

void Game::DrawGame() const
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
    else if (currentState == GameState::PAUSED)
    {
        // 可选：绘制一个半透明的覆盖层使背景变暗，以突出暂停信息
        DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.4f));

        const auto pauseText = "PAUSED";
        constexpr int pauseTextFontSize = 30; // 字体大小
        constexpr auto pauseTextColor = RAYWHITE; // 文字颜色
        const int textWidth = MeasureText(pauseText, pauseTextFontSize);

        // 定位在右下角
        const float posX = virtualScreenWidth - textWidth - 20; // 20是屏幕右边距
        const float posY = virtualScreenHeight - static_cast<float>(pauseTextFontSize) - 20; // 20是屏幕下边距

        // 绘制文字
        DrawText(pauseText, static_cast<int>(posX),
                 static_cast<int>(posY + (static_cast<float>(pauseTextFontSize) - pauseTextFontSize) / 2.0f),
                 pauseTextFontSize, pauseTextColor); // 垂直居中文字
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
    const float scale = std::min(static_cast<float>(screenWidth) / virtualScreenWidth,
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
        const int randIdx = rand() % roadSegmentTextures.size();
        const Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
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
        const int randIdx = rand() % roadSegmentTextures.size();
        const Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
        activeRoadSegments.push_back({chosenRoadTex, rightmostX});
        rightmostX += chosenRoadTex.width;
    }
}

void Game::DrawRoadSegments() const
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
                    SeekMusicStream(bgmMusic, 0.0f);
                }
            }
        }

        const float deltaTime = GetFrameTime();
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
