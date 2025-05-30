#include "../include/Game.h"
#include <iostream>
#include <algorithm>

Game::Game(const int width, const int height, const char* title)
    : screenWidth(width), screenHeight(height),
      targetRenderTexture{},
      sourceRec{0.0f, 0.0f, static_cast<float>(virtualScreenWidth), static_cast<float>(virtualScreenHeight)},
      destRec{0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)},
      origin{0.0f, 0.0f}, isFullscreen(false),
      windowedPosX(0), windowedPosY(0),
      windowedWidth(width), windowedHeight(height),
      dino(nullptr), playerSword(nullptr),
      currentState(GameState::PLAYING),
      groundY(0),
      timePlayed(0.0f),
      score(0),
      worldBaseScrollSpeed(400.0f),
      currentWorldScrollSpeed(worldBaseScrollSpeed),
      worldSpeedIncreaseRate(10.0f),
      obstacleSpawnTimer(0.0f),
      minObstacleSpawnInterval(0.3f), maxObstacleSpawnInterval(1.8f),
      currentObstacleSpawnInterval(0.0f),
      dinoDeadTexture{},
      cloudTexture{}, swordTexture{},
      jumpSound{nullptr},
      dashSound{nullptr},
      deadSound{nullptr},
      bombSound{nullptr},
      swordSound{nullptr},
      screamSound{nullptr},
      bgmMusic{nullptr},
      birdDeathParticles(300)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, title);
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    nextCloudSpawnTime = randF(1, 6);
    const auto [x, y] = GetWindowPosition();
    windowedPosX = static_cast<int>(x);
    windowedPosY = static_cast<int>(y);
    SetWindowMinSize(virtualScreenWidth / 2, virtualScreenHeight / 2);
    SetTargetFPS(160);
    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_POINT);
    groundY = static_cast<float>(virtualScreenHeight) * 0.85f;
    LoadResources();
    birdDeathParticleProps.lifeTimeMin = 1.5f;
    birdDeathParticleProps.lifeTimeMax = 3.5f;
    birdDeathParticleProps.initialSpeedMin = 50.0f;
    birdDeathParticleProps.initialSpeedMax = 150.0f;
    birdDeathParticleProps.emissionAngleMin = 0.0f;
    birdDeathParticleProps.emissionAngleMax = 360.0f;
    birdDeathParticleProps.startSizeMin = 1.5f;
    birdDeathParticleProps.startSizeMax = 8.0f;
    birdDeathParticleProps.startColor = {200, 80, 80, 200};
    birdDeathParticleProps.angularVelocityMin = -180.0f;
    birdDeathParticleProps.angularVelocityMax = 180.0f;
    birdDeathParticleProps.gravityScaleMin = 0.5f;
    birdDeathParticleProps.gravityScaleMax = 1.5f;
    birdDeathParticleProps.targetGroundY = groundY + 5.0f;
    birdDeathParticles.SetGravity({0, 800.0f});
    instructionManager.Initialize(virtualScreenWidth, groundY, bombSound);
    InitGame();
    HandleWindowResize();
}

Game::~Game()
{
    UnloadRenderTexture(targetRenderTexture);
    UnloadResources();
    delete dino;
    delete playerSword;
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

    if (const Texture2D tempSwordTex = LoadTexture("assets/images/sword.png"); tempSwordTex.id > 0)
    {
        SetTextureFilter(tempSwordTex, TEXTURE_FILTER_POINT);
        swordTexture = tempSwordTex;
    }
    if (const Texture2D tempTex = LoadTexture("assets/images/dino_dead.png"); tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        dinoDeadTexture = tempTex;
    }
    if (const Texture2D tempTex = LoadTexture("assets/images/cloud.png"); tempTex.id > 0)
    {
        SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
        cloudTexture = tempTex;
    }

    auto LoadTextures = [](const std::vector<std::string>& paths, std::vector<Texture2D>& container)
    {
        for (const auto& path : paths)
        {
            if (Texture2D tempTex = LoadTexture(path.c_str()); tempTex.id > 0)
            {
                SetTextureFilter(tempTex, TEXTURE_FILTER_POINT);
                container.push_back(tempTex);
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
    };

    LoadSoundEffect("assets/sounds/jump.wav", jumpSound);
    LoadSoundEffect("assets/sounds/dash.wav", dashSound);
    LoadSoundEffect("assets/sounds/dead.wav", deadSound);
    LoadSoundEffect("assets/sounds/bomb.wav", bombSound);
    LoadSoundEffect("assets/sounds/scream.wav", screamSound);
    LoadSoundEffect("assets/sounds/sword.wav", swordSound);
    bgmMusic = LoadMusicStream("assets/sounds/bgm.wav");
    SetMusicVolume(bgmMusic, 0.3f);
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
    if (swordTexture.id > 0) UnloadTexture(swordTexture);
    if (swordSound.frameCount > 0) UnloadSound(swordSound);
    if (jumpSound.frameCount > 0) UnloadSound(jumpSound);
    if (dashSound.frameCount > 0) UnloadSound(dashSound);
    if (deadSound.frameCount > 0) UnloadSound(deadSound);
    if (bombSound.frameCount > 0) UnloadSound(bombSound);
    if (screamSound.frameCount > 0) UnloadSound(screamSound);
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
    delete playerSword;
    if (dino && swordTexture.id > 0 && swordSound.frameCount > 0)
    {
        playerSword = new Sword(swordTexture, swordSound, dino);
    }
    else
    {
        playerSword = nullptr;
    }


    obstacles.clear();
    birds.clear();
    activeClouds.clear();
    cloudSpawnTimerValue = 0.0f;
    nextCloudSpawnTime = randF(1.0f, 6.0f);

    score = 0;
    timePlayed = 0.0f;
    worldBaseScrollSpeed = 200.0f;
    currentWorldScrollSpeed = worldBaseScrollSpeed;
    obstacleSpawnTimer = 0.0f;
    currentObstacleSpawnInterval = randF(minObstacleSpawnInterval, maxObstacleSpawnInterval);

    InitRoads();

    currentState = GameState::PLAYING;
    instructionManager.ResetAllInstructions();


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
        }
        else if (currentState == GameState::PAUSED)
        {
            currentState = GameState::PLAYING;
            if (bgmMusic.frameCount > 0)
            {
                ResumeMusicStream(bgmMusic);
            }
        }
    }
    if (currentState == GameState::PAUSED)
    {
        Vector2 virtualMousePos = {0, 0};
        Vector2 mousePoint = GetMousePosition();
        virtualMousePos.x = (mousePoint.x - destRec.x) / (destRec.width / virtualScreenWidth);
        virtualMousePos.y = (mousePoint.y - destRec.y) / (destRec.height / virtualScreenHeight);


        float buttonWidth = 220;
        float buttonHeight = 50;
        float buttonSpacing = 20;

        float restartButtonY = virtualScreenHeight * 0.45f;

        Rectangle current_restartButtonRect = {
            virtualScreenWidth / 2.0f - buttonWidth / 2.0f,
            restartButtonY,
            buttonWidth,
            buttonHeight
        };

        Rectangle current_exitButtonRect = {
            virtualScreenWidth / 2.0f - buttonWidth / 2.0f,
            current_restartButtonRect.y + buttonHeight + buttonSpacing,
            buttonWidth,
            buttonHeight
        };


        if (CheckCollisionPointRec(virtualMousePos, current_restartButtonRect))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                ResetGame();
            }
        }


        if (CheckCollisionPointRec(virtualMousePos, current_exitButtonRect))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                CloseWindow();
            }
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
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && playerSword)
        {
            playerSword->Attack();
        }
    }
}

void Game::UpdateGame(const float deltaTime)
{
    instructionManager.Update(deltaTime, currentWorldScrollSpeed, timePlayed);

    if (currentState == GameState::GAME_OVER || currentState == GameState::PAUSED)
    {
        birdDeathParticles.Update(deltaTime);
        if (playerSword) playerSword->Update(deltaTime);
        return;
    }

    timePlayed += deltaTime;
    score = static_cast<int>(timePlayed * 10);

    worldBaseScrollSpeed += worldSpeedIncreaseRate * deltaTime;
    currentWorldScrollSpeed = worldBaseScrollSpeed;

    dino->Update(deltaTime, currentWorldScrollSpeed);
    if (playerSword) playerSword->Update(deltaTime);

    if (dino->position.x < 0) dino->position.x = 0;
    if (dino->position.x + dino->GetWidth() > virtualScreenWidth)
    {
        dino->position.x = virtualScreenWidth - dino->GetWidth();
    }

    UpdateRoadSegments(deltaTime);
    UpdateClouds(deltaTime);
    birdDeathParticles.Update(deltaTime);

    for (auto it = obstacles.begin(); it != obstacles.end();)
    {
        it->setSpeed(currentWorldScrollSpeed);
        it->Update(deltaTime);
        if (it->IsOffScreen())it = obstacles.erase(it);
        else ++it;
    }
    for (auto it = birds.begin(); it != birds.end();)
    {
        const float birdSpeedFactor = randF(0.3f, 2.51f);
        it->setSpeed(currentWorldScrollSpeed * birdSpeedFactor);
        it->Update(deltaTime);
        if (it->IsOffScreen()) it = birds.erase(it);
        else ++it;
    }

    obstacleSpawnTimer += deltaTime;
    if (obstacleSpawnTimer >= currentObstacleSpawnInterval)
    {
        SpawnObstacleOrBird();
        obstacleSpawnTimer = 0.0f;
        currentObstacleSpawnInterval = randF(minObstacleSpawnInterval, maxObstacleSpawnInterval);
    }

    cloudSpawnTimerValue += deltaTime;
    if (cloudSpawnTimerValue >= nextCloudSpawnTime)
    {
        SpawnCloud();
        cloudSpawnTimerValue = 0.0f;
        nextCloudSpawnTime = randF(1, 6);
    }

    CheckCollisions();
}

void Game::SpawnObstacleOrBird()
{
    if (birdFrames.empty() && smallCactusTextures.empty() && bigCactusTextures.empty()) return;
    float spawnX = static_cast<float>(virtualScreenWidth) + 250.0f + randF(0, 350);

    if (const int entityTypeRoll = randI(0, 100); entityTypeRoll < 40)
    {
        Texture2D chosenCactusTex;
        if (const bool preferSmall = (randI(0, 3) != 0 && !smallCactusTextures.empty()) || bigCactusTextures.empty();
            preferSmall && !smallCactusTextures.empty())
            chosenCactusTex = smallCactusTextures[randI(0, smallCactusTextures.size())];
        else if (!bigCactusTextures.empty()) chosenCactusTex = bigCactusTextures[randI(0, bigCactusTextures.size())];
        else if (!smallCactusTextures.empty())
            chosenCactusTex = smallCactusTextures[randI(0, smallCactusTextures.size())];
        else return;
        if (chosenCactusTex.id > 0) obstacles.emplace_back(spawnX, groundY, currentWorldScrollSpeed, chosenCactusTex);
    }
    else
    {
        if (!birdFrames.empty())
        {
            const float birdSpriteHeight = birdFrames[0].height;
            float spawnY;
            float y_spawn_upper_limit = virtualScreenHeight * 0.3f;
            float y_spawn_lower_limit = groundY - birdSpriteHeight;
            if (y_spawn_upper_limit >= y_spawn_lower_limit)
            {
                spawnY = std::min(y_spawn_upper_limit, y_spawn_lower_limit);
            }
            else
            {
                spawnY = randF(y_spawn_upper_limit, y_spawn_lower_limit);
            }
            spawnY = std::max(spawnY, 0.0f);
            spawnY = std::min(spawnY, groundY - birdSpriteHeight);
            birds.emplace_back(spawnX, spawnY, currentWorldScrollSpeed, birdFrames);
        }
    }
}

void Game::CheckCollisions()
{
    if (!dino) return;

    const Rectangle dinoRect = dino->GetCollisionRect();
    bool dinoHitSomething = false;


    for (const auto& obs : obstacles)
    {
        if (CheckCollisionRecs(dinoRect, obs.GetCollisionRect()))
        {
            dinoHitSomething = true;
            break;
        }
    }


    if (!dinoHitSomething)
    {
        for (const auto& brd : birds)
        {
            if (CheckCollisionRecs(dinoRect, brd.GetCollisionRect()))
            {
                dinoHitSomething = true;
                break;
            }
        }
    }


    if (!dinoHitSomething)
    {
        for (const auto& instructionRect : instructionManager.GetAllActiveCollidableInstructionRects())
        {
            if (CheckCollisionRecs(dinoRect, instructionRect))
            {
                dinoHitSomething = true;

                break;
            }
        }
    }


    if (playerSword && playerSword->IsAttacking())
    {
        playerSword->CheckCollisionsWithBirds(birds, score,
                                              birdDeathParticles, birdDeathParticleProps,
                                              currentWorldScrollSpeed,
                                              this->screamSound);
    }


    if (dinoHitSomething)
    {
        currentState = GameState::GAME_OVER;
        dino->MarkAsDead();
        if (bgmMusic.frameCount > 0 && IsMusicStreamPlaying(bgmMusic))
        {
            StopMusicStream(bgmMusic);
        }
        if (deadSound.frameCount > 0 && IsAudioDeviceReady())
        {
            PlaySound(deadSound);
        }
    }
}

void Game::DrawGame() const
{
    BeginTextureMode(targetRenderTexture);
    ClearBackground(RAYWHITE);


    for (const auto& cloud : activeClouds)
    {
        cloud.Draw();
    }

    for (const auto& [texture, xPosition] : activeRoadSegments)
    {
        DrawTexture(texture, static_cast<int>(xPosition), static_cast<int>(groundY), WHITE);
    }

    if (dino)
    {
        dino->Draw();


        if (playerSword && playerSword->IsOnCooldown())
        {
            const float dinoDrawX = dino->position.x;
            const float dinoDrawY = dino->position.y;
            const float dinoDrawWidth = dino->GetWidth();

            const float cdBarMaxWidth = dinoDrawWidth * 0.7f;
            const float cdBarHeight = 7.0f;
            const float cdBarOffsetY = 12.0f;

            float cooldownProgress = playerSword->GetCooldownProgress();
            float currentCDBarWidth = cdBarMaxWidth * cooldownProgress;

            Vector2 cdBarBgPosition = {
                dinoDrawX + (dinoDrawWidth / 2.0f) - (cdBarMaxWidth / 2.0f),
                dinoDrawY - cdBarHeight - cdBarOffsetY
            };


            DrawRectangleV(cdBarBgPosition, {cdBarMaxWidth, cdBarHeight}, Fade(DARKGRAY, 0.75f));


            DrawRectangleV(cdBarBgPosition, {currentCDBarWidth, cdBarHeight}, LIGHTGRAY);


            DrawRectangleLinesEx({cdBarBgPosition.x, cdBarBgPosition.y, cdBarMaxWidth, cdBarHeight}, 1.0f, BLACK);
        }
    }

    for (const auto& obs : obstacles)
    {
        obs.Draw();
    }

    for (const auto& brd : birds)
    {
        brd.Draw();
    }

    if (playerSword)
    {
        playerSword->Draw();
    }

    birdDeathParticles.Draw();


    DrawText(TextFormat("Score: %06d", score), 20, 20, 30, DARKGRAY);
    DrawText(TextFormat("Time: %.1fs", timePlayed),
             virtualScreenWidth - MeasureText(TextFormat("Time: %.1fs", timePlayed), 20) - 20, 20, 20, DARKGRAY);

    instructionManager.Draw();

    if (currentState == GameState::GAME_OVER)
    {
        DrawText("GAME OVER", virtualScreenWidth / 2 - MeasureText("GAME OVER", 70) / 2, virtualScreenHeight * 0.4f, 70,
                 RED);
        DrawText("Press R to Restart",
                 virtualScreenWidth / 2 - MeasureText("Press R to Restart", 25) / 2,
                 virtualScreenHeight * 0.6f, 25, DARKGRAY);
    }
    else if (currentState == GameState::PAUSED)
    {
        DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.8f));


        const char* gameTitleText = "Dino Plus Ultra";
        int titleFontSize = 60;
        int titleTextWidth = MeasureText(gameTitleText, titleFontSize);
        DrawText(gameTitleText, virtualScreenWidth / 2 - titleTextWidth / 2, virtualScreenHeight * 0.2f, titleFontSize,
                 WHITE);


        float buttonWidth = 220;
        float buttonHeight = 50;
        int buttonFontSize = 25;
        float buttonSpacing = 20;

        Vector2 virtualMousePos = {0, 0};


        Vector2 mousePoint = GetMousePosition();
        virtualMousePos.x = (mousePoint.x - destRec.x) / (destRec.width / virtualScreenWidth);
        virtualMousePos.y = (mousePoint.y - destRec.y) / (destRec.height / virtualScreenHeight);


        const char* restartText = "Restart Game";


        Rectangle localRestartButtonRect = {
            virtualScreenWidth / 2.0f - buttonWidth / 2.0f,
            virtualScreenHeight * 0.45f,
            buttonWidth,
            buttonHeight
        };


        Color restartButtonColor = DARKGRAY;
        if (CheckCollisionPointRec(virtualMousePos, localRestartButtonRect))
        {
            restartButtonColor = GRAY;
        }
        DrawRectangleRec(localRestartButtonRect, restartButtonColor);
        DrawRectangleLinesEx(localRestartButtonRect, 2, LIGHTGRAY);
        int restartTextWidth = MeasureText(restartText, buttonFontSize);
        DrawText(restartText, localRestartButtonRect.x + (buttonWidth - restartTextWidth) / 2,
                 localRestartButtonRect.y + (buttonHeight - buttonFontSize) / 2, buttonFontSize, WHITE);


        const char* exitText = "Quit Game";
        Rectangle localExitButtonRect = {
            virtualScreenWidth / 2.0f - buttonWidth / 2.0f,
            localRestartButtonRect.y + buttonHeight + buttonSpacing,
            buttonWidth,
            buttonHeight
        };


        Color exitButtonColor = DARKGRAY;
        if (CheckCollisionPointRec(virtualMousePos, localExitButtonRect))
        {
            exitButtonColor = GRAY;
        }
        DrawRectangleRec(localExitButtonRect, exitButtonColor);
        DrawRectangleLinesEx(localExitButtonRect, 2, LIGHTGRAY);
        int exitTextWidth = MeasureText(exitText, buttonFontSize);
        DrawText(exitText, localExitButtonRect.x + (buttonWidth - exitTextWidth) / 2,
                 localExitButtonRect.y + (buttonHeight - buttonFontSize) / 2, buttonFontSize, WHITE);
    }
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(targetRenderTexture.texture, sourceRec, destRec, origin, 0.0f, WHITE);
    EndDrawing();
}

void Game::SpawnCloud()
{
    Vector2 initialPosition;
    initialPosition.x = static_cast<float>(virtualScreenWidth) + randF(50, cloudTexture.width * 2);
    initialPosition.y = randF(virtualScreenHeight / 8, virtualScreenHeight / 2);
    float cloudSpeed = randF(15, 45) + currentWorldScrollSpeed * 0.05f;
    activeClouds.emplace_back(cloudTexture, initialPosition, cloudSpeed);
}


void Game::UpdateClouds(const float deltaTime)
{
    for (auto it = activeClouds.begin(); it != activeClouds.end();)
    {
        it->Update(deltaTime);
        if (it->IsOffScreen())
        {
            it = activeClouds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Game::ResetGame()
{
    if (bgmMusic.frameCount > 0 && IsMusicStreamPlaying(bgmMusic))
    {
        StopMusicStream(bgmMusic);
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
    InitRoads();
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

void Game::InitRoads()
{
    activeRoadSegments.clear();
    if (roadSegmentTextures.empty() || roadSegmentTextures[0].id == 0) return;
    float currentX = 0.0f;
    while (currentX < virtualScreenWidth * 1.5f)
    {
        const int randIdx = randI(0, roadSegmentTextures.size());
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
        InitRoads();
        if (!activeRoadSegments.empty())
        {
            rightmostX = activeRoadSegments.back().xPosition + activeRoadSegments.back().texture.width;
        }
        else return;
    }
    while (rightmostX < virtualScreenWidth * 1.5f)
    {
        const int randIdx = randI(0, roadSegmentTextures.size());
        const Texture2D chosenRoadTex = roadSegmentTextures[randIdx];
        activeRoadSegments.push_back({chosenRoadTex, rightmostX});
        rightmostX += chosenRoadTex.width;
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
            if (IsKeyPressed(KEY_R))
            {
                ResetGame();
            }
        }
        DrawGame();
    }
}
