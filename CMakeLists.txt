cmake_minimum_required(VERSION 3.16)
project(FantasyCraft LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags for high performance release build
if(MSVC)
    add_compile_options(/O2 /fp:fast)
else()
    add_compile_options(-O3 -march=native -ffast-math)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(SOURCES
    src/glad.c
    src/main.cpp
    src/engine/Shader.cpp
    src/engine/Camera.cpp
    src/engine/TextureAtlas.cpp
    src/engine/Raycast.cpp
    src/engine/Player.cpp
    src/world/Chunk.cpp
    src/world/ChunkMesher.cpp
    src/world/IslandGenerator.cpp
    src/world/World.cpp
    src/world/MobManager.cpp
    src/world/BlockParticles.cpp
    src/ui/HUD.cpp
)

add_executable(fantasy_craft ${SOURCES})

# Target output directory directly in root folder so it accesses assets/ directly
set_target_properties(fantasy_craft PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_NAME "fantasy_craft"
)

# Link GLFW and Windows OpenGL libraries
target_link_directories(fantasy_craft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(fantasy_craft PRIVATE
    glfw3
    opengl32
    gdi32
    user32
    kernel32
)
