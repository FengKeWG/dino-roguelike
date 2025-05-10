cmake_minimum_required(VERSION 3.30)
project(DinoRoguelike)

set(CMAKE_CXX_STANDARD 26)

# Raylib 库的路径（根据你的项目结构调整）
set(RAYLIB_PATH ${CMAKE_SOURCE_DIR}/libs/raylib)

# 包含 raylib 头文件目录
include_directories(${RAYLIB_PATH}/include)

# 添加可执行文件
add_executable(DinoRoguelike src/main.cpp
        src/Dinosaur.cpp
        include/Dinosaur.h
        src/Obstacle.cpp
        include/Obstacle.h
        src/Game.cpp
        include/Game.h
        src/Bird.cpp
        include/Bird.h
        src/ParticleSystem.cpp
        include/ParticleSystem.h
        src/InstructionText.cpp
        include/InstructionText.h
        src/InstructionManager.cpp
        include/InstructionManager.h
        src/Cloud.cpp
        include/Cloud.h
)

# 链接 raylib 库
target_link_libraries(DinoRoguelike ${RAYLIB_PATH}/lib/libraylib.a)

#如果需要windows的支持，加上windows
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(DinoRoguelike "-lopengl32" "-lgdi32" "-lwinmm")
endif ()