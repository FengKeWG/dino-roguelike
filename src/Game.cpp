#include "../include/Game.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

Game::Game(const int width, const int height, const char* title)
    : screenWidth(width), screenHeight(height),
      targetRenderTexture{},
      sourceRec{0.0f, 0.0f, static_cast<float>(virtualScreenWidth), static_cast<float>(virtualScreenHeight)},
      destRec{0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)},
      origin{0.0f, 0.0f}, isFullscreen(false),
      windowedPosX(0), windowedPosY(0),
      windowedWidth(width), windowedHeight(height),
      dino(nullptr),
      currentState(GameState::PLAYING),
      groundY(0),
      timePlayed(0.0f),
      score(0),
      worldBaseScrollSpeed(400.0f),
      currentWorldScrollSpeed(worldBaseScrollSpeed),
      worldSpeedIncreaseRate(10.0f),
      obstacleSpawnTimer(0.0f),
      minObstacleSpawnInterval(0.6f), maxObstacleSpawnInterval(2.4f),
      currentObstacleSpawnInterval(0.0f),
      dinoDeadTexture{},
      cloudTexture{},
      jumpSound{nullptr},
      dashSound{nullptr},
      deadSound{nullptr},
      bombSound{nullptr},
      bgmMusic{nullptr},
      cloudSpawnTimerValue(0.0f)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, title);
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    nextCloudSpawnTime = static_cast<float>(GetRandomValue(10, 60)) / 10.0f;
    const auto [x, y] = GetWindowPosition();
    windowedPosX = static_cast<int>(x);
    windowedPosY = static_cast<int>(y);
    SetWindowMinSize(virtualScreenWidth / 2, virtualScreenHeight / 2);
    SetTargetFPS(160);
    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_POINT);
    groundY = static_cast<float>(virtualScreenHeight) * 0.85f; // 先计算好 groundY
    LoadResources();
    instructionManager.Initialize(virtualScreenWidth, groundY, bombSound);
    InitGame(); // InitGame 会设置 currentState 为 PAUSED
    HandleWindowResize();
}

Game::~Game()
{
    UnloadRenderTexture(targetRenderTexture);
    UnloadResources();
    delete dino;
    CloseAudioDevice();
    CloseWindow();
}

void Game::LoadResources()
{
    dinoRunFrames.clear();
    dinoSneakFrames.clear();
    smallCactusTextures.clear();
    bigCactusTextures.clear();
    roadSegmentTextures.clear();
    birdFrames.clear();

    if (const Texture2D tempTex = LoadTexture("assets/images/dino_dead.png"); tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        dinoDeadTexture = tempTex;
    }
    else { TraceLog(LOG_WARNING, "Failed: assets/images/dino_dead.png"); }

    if (const Texture2D tempTex = LoadTexture("assets/images/cloud.png"); tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        cloudTexture = tempTex;
    }
    else { TraceLog(LOG_WARNING, "Failed: assets/images/cloud.png"); }

    auto LoadTextures = [](const std::vector<std::string>& paths, std::vector<Texture2D>& container)
    {
        for (const auto& path : paths)
        {
            if (Texture2D tempTex = LoadTexture(path.c_str()); tempTex.id > 0)
            {
                SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
                container.push_back(tempTex);
            }
            else
            {
                TraceLog(LOG_WARNING, ("Failed: " + path).c_str());
            }
        }
    };

    LoadTextures({
                     "assets/images/dino_run_1.png",
                     "assets/images/dino_run_2.png"
                 }, dinoRunFrames);
    LoadTextures({
                     "assets/images/dino_sneak_1.png",
                     "assets/images/dino_sneak_2.png"
                 }, dinoSneakFrames);
    LoadTextures({
                     "assets/images/small_cactus_1.png",
                     "assets/images/small_cactus_2.png",
                     "assets/images/small_cactus_3.png"
                 }, smallCactusTextures);
    LoadTextures({
                     "assets/images/big_cactus_1.png",
                     "assets/images/big_cactus_2.png"
                 }, bigCactusTextures);
    LoadTextures({
                     "assets/images/road_1.png",
                     "assets/images/road_2.png",
                     "assets/images/road_3.png",
                     "assets/images/road_4.png"
                 }, roadSegmentTextures);
    LoadTextures({
                     "assets/images/bird_1.png",
                     "assets/images/bird_2.png"
                 }, birdFrames);

    auto LoadSoundEffect = [](const char* path, Sound& sound)
    {
        sound = LoadSound(path);
        if (sound.frameCount == 0)
        {
            TraceLog(LOG_WARNING, ("Failed to load sound: " + std::string(path)).c_str());
        }
    };

    LoadSoundEffect("assets/sounds/jump.wav", jumpSound);
    LoadSoundEffect("assets/sounds/dash.wav", dashSound);
    LoadSoundEffect("assets/sounds/dead.wav", deadSound);
    LoadSoundEffect("assets/sounds/bomb.wav", bombSound);

    bgmMusic = LoadMusicStream("assets/sounds/bgm.wav");
    if (bgmMusic.frameCount == 0) { TraceLog(LOG_WARNING, "Failed to load music stream: assets/sounds/bgm.wav"); }
    else { SetMusicVolume(bgmMusic, 0.3f); }

    if (dinoRunFrames.empty()) { TraceLog(LOG_ERROR, "CRITICAL: Dinosaur has no run frames for animation."); }
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
    if (dinoDeadTexture.id > 0) UnloadTexture(dinoDeadTexture);
    if (cloudTexture.id > 0) UnloadTexture(cloudTexture);

    if (jumpSound.frameCount > 0) UnloadSound(jumpSound);
    if (dashSound.frameCount > 0) UnloadSound(dashSound);
    if (deadSound.frameCount > 0) UnloadSound(deadSound);
    if (bombSound.frameCount > 0) UnloadSound(bombSound);
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
    dino = new Dinosaur(virtualScreenWidth / 4.0f, groundY,
                        dinoRunFrames, dinoSneakFrames,
                        dinoDeadTexture,
                        jumpSound, dashSound);

    obstacles.clear();
    birds.clear();
    activeClouds.clear();
    cloudSpawnTimerValue = 0.0f;
    nextCloudSpawnTime = static_cast<float>(GetRandomValue(10, 60)) / 10.0f;
    score = 0;
    timePlayed = 0.0f;
    worldBaseScrollSpeed = 200.0f;
    currentWorldScrollSpeed = worldBaseScrollSpeed;
    obstacleSpawnTimer = 0.0f;
    currentObstacleSpawnInterval = minObstacleSpawnInterval + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX
        / (maxObstacleSpawnInterval - minObstacleSpawnInterval)));

    InitRoadSegments();
    currentState = GameState::PLAYING;
    instructionManager.ResetAllInstructions();
    instructionManager.RequestShowInstruction("jump_tip");
    instructionManager.RequestShowInstruction("move_tip");
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
        if (!isFullscreen)
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
            isFullscreen = true;
        }
        else
        {
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            SetWindowSize(windowedWidth, windowedHeight);
            SetWindowPosition(windowedPosX, windowedPosY);
            SetWindowState(FLAG_WINDOW_RESIZABLE);
            isFullscreen = false;
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
    instructionManager.Update(deltaTime);

    if (currentState == GameState::GAME_OVER || currentState == GameState::PAUSED)
    {
        return;
    }

    timePlayed += deltaTime;
    score = static_cast<int>(timePlayed * 10);

    worldBaseScrollSpeed += worldSpeedIncreaseRate * deltaTime;
    currentWorldScrollSpeed = worldBaseScrollSpeed;

    dino->Update(deltaTime, currentWorldScrollSpeed);

    // 恐龙边界检查 (与之前相同)
    if (dino->position.x < 0) dino->position.x = 0;
    if (dino->position.x + dino->GetWidth() > virtualScreenWidth)
    {
        dino->position.x = virtualScreenWidth - dino->GetWidth();
    }

    UpdateRoadSegments(deltaTime); // 路面滚动
    UpdateClouds(deltaTime);

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
        currentObstacleSpawnInterval = minObstacleSpawnInterval + static_cast<float>(rand()) / (RAND_MAX / (
            maxObstacleSpawnInterval - minObstacleSpawnInterval));
    }

    cloudSpawnTimerValue += deltaTime;
    if (cloudSpawnTimerValue >= nextCloudSpawnTime)
    {
        SpawnCloud();
        cloudSpawnTimerValue = 0.0f;
        nextCloudSpawnTime = static_cast<float>(GetRandomValue(10, 60)) / 10.0f;
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
    const Rectangle dinoRect = dino->GetCollisionRect();
    bool collisionDetected = false;

    for (const auto& obs : obstacles)
    {
        if (CheckCollisionRecs(dinoRect, obs.GetCollisionRect()))
        {
            collisionDetected = true;
            break;
        }
    }
    if (!collisionDetected)
    {
        for (const auto& brd : birds)
        {
            if (CheckCollisionRecs(dinoRect, brd.GetCollisionRect()))
            {
                collisionDetected = true;
                break;
            }
        }
    }

    if (collisionDetected)
    {
        currentState = GameState::GAME_OVER;
        dino->MarkAsDead(); // <--- 标记恐龙死亡
        if (bgmMusic.frameCount > 0 && IsMusicStreamPlaying(bgmMusic))
        {
            StopMusicStream(bgmMusic); // <--- 停止BGM
        }
        if (deadSound.frameCount > 0 && IsAudioDeviceReady())
        {
            PlaySound(deadSound); // <--- 播放死亡音效
        }
    }
}

void Game::DrawGame() const
{
    BeginTextureMode(targetRenderTexture);
    ClearBackground(RAYWHITE);
    DrawClouds();
    DrawRoads();
    if (dino) dino->Draw();
    for (auto& obs : obstacles) obs.Draw();
    for (auto& brd : birds) brd.Draw();
    DrawText(TextFormat("Score: %06d", score), 20, 20, 30, DARKGRAY);
    DrawText(TextFormat("Time: %.1fs", timePlayed),
             virtualScreenWidth - MeasureText(TextFormat("Time: %.1fs", timePlayed), 20) - 20, 20, 20, DARKGRAY);
    instructionManager.Draw();
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
        DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.4f));

        const auto pauseText = "PAUSED";
        constexpr int pauseTextFontSize = 30;
        constexpr auto pauseTextColor = RAYWHITE;
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

void Game::SpawnCloud()
{
    if (cloudTexture.id <= 0) return;
    Cloud newCloud;
    newCloud.texture = cloudTexture;
    newCloud.position.x = static_cast<float>(virtualScreenWidth) + GetRandomValue(50, cloudTexture.width * 2);
    newCloud.position.y = static_cast<float>(GetRandomValue(virtualScreenHeight / 8, virtualScreenHeight / 2));
    newCloud.speed = static_cast<float>(GetRandomValue(15, 45)) + currentWorldScrollSpeed * 0.05f;
    activeClouds.push_back(newCloud);
}

void Game::UpdateClouds(const float deltaTime)
{
    for (auto it = activeClouds.begin(); it != activeClouds.end();)
    {
        it->position.x -= it->speed * deltaTime;
        if (it->position.x + it->texture.width < 0)
        {
            it = activeClouds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Game::DrawClouds() const
{
    for (const auto& cloud : activeClouds)
    {
        DrawTextureV(cloud.texture, cloud.position, WHITE);
    }
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
    if (isFullscreen &&
        (GetScreenWidth() != GetMonitorWidth(GetCurrentMonitor()) ||
            GetScreenHeight() != GetMonitorHeight(GetCurrentMonitor())))
    {
        isFullscreen = false;
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
    InitRoadSegments();
    TraceLog(LOG_INFO, "Window resized/state changed: Phys: %dx%d, Virt: %dx%d. Render scale: %.2f. FakeFullscreen: %s",
             screenWidth, screenHeight, virtualScreenWidth, virtualScreenHeight, destRec.width / virtualScreenWidth,
             isFullscreen ? "ON" : "OFF");
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
    for (auto& [texture, xPosition] : activeRoadSegments)
    {
        xPosition -= currentWorldScrollSpeed * deltaTime;
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

void Game::DrawRoads() const
{
    if (roadSegmentTextures.empty())
    {
        DrawLine(0, static_cast<int>(groundY), virtualScreenWidth, static_cast<int>(groundY), DARKGRAY);
        return;
    }
    for (const auto& [texture, xPosition] : activeRoadSegments)
    {
        DrawTexture(texture, static_cast<int>(xPosition), static_cast<int>(groundY), WHITE);
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
