#include "engine/Common.h"
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "engine/Shader.h"
#include "engine/Camera.h"
#include "engine/Player.h"
#include "world/World.h"
#include "world/MobManager.h"
#include "world/BlockParticles.h"
#include "ui/HUD.h"

// Window dimensions
int g_windowWidth = 1280;
int g_windowHeight = 720;

// Global pointers for GLFW callbacks
Player* g_player = nullptr;
World* g_world = nullptr;
MobManager* g_mobs = nullptr;
HUD* g_hud = nullptr;
bool g_cursorCaptured = true;
bool g_firstMouse = true;
double g_lastMouseX = 640.0;
double g_lastMouseY = 360.0;

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    g_windowWidth = width;
    g_windowHeight = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!g_cursorCaptured) return;

    if (g_firstMouse) {
        g_lastMouseX = xpos;
        g_lastMouseY = ypos;
        g_firstMouse = false;
    }

    float xoffset = (float)(xpos - g_lastMouseX);
    float yoffset = (float)(g_lastMouseY - ypos); // Invert Y for OpenGL
    g_lastMouseX = xpos;
    g_lastMouseY = ypos;

    if (g_player) {
        g_player->camera.processMouseMovement(xoffset, yoffset);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (!g_cursorCaptured) {
        if (action == GLFW_PRESS && g_player && g_world && g_hud &&
            (g_player->inventoryOpen || g_player->chestOpen || g_player->craftingOpen)) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            g_hud->handleClick(mouseX, mouseY, button, *g_player, *g_world, g_windowWidth, g_windowHeight);
            return;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            g_cursorCaptured = true;
            g_firstMouse = true;
        }
        return;
    }

    if (g_player && g_world) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            RaycastResult target = g_player->getLookTarget(*g_world);
            if (target.hit && target.blockId == BLOCK_CHEST && g_player->openChestAt(target.blockPos, *g_world)) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                g_cursorCaptured = false;
                g_firstMouse = true;
                return;
            }
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && g_mobs) {
            if (g_player->selectedTool == ToolType::Bow) {
                if (g_player->consumeArrow() &&
                    g_mobs->tryHit(g_player->camera.position, g_player->camera.front,
                                   g_player->getAttackDamage(), 40.0f, g_world))
                    g_player->registerHit();
                return;
            }
            if (g_mobs->tryHit(g_player->camera.position, g_player->camera.front,
                               g_player->getAttackDamage(), 6.0f, g_world)) {
                g_player->registerHit();
                return;
            }
        }
        if (g_player->selectedTool == ToolType::Bow && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            return;
        }
        g_player->handleMouseClick(button, action, *g_world);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (g_player && g_cursorCaptured) {
        g_player->handleScroll(yoffset);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            if (g_player && (g_player->craftingOpen || g_player->inventoryOpen || g_player->chestOpen)) {
                g_player->closeMenus();
                g_cursorCaptured = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                g_firstMouse = true;
            } else {
                g_cursorCaptured = !g_cursorCaptured;
                if (g_cursorCaptured) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    g_firstMouse = true;
                } else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }
        } else if (key == GLFW_KEY_E && g_player) {
            if (g_player->inventoryOpen || g_player->chestOpen) {
                g_player->closeMenus();
                g_cursorCaptured = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                g_firstMouse = true;
            } else {
                g_player->closeMenus();
                g_player->inventoryOpen = true;
                g_cursorCaptured = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                g_firstMouse = true;
            }
        } else if (key == GLFW_KEY_C && g_player) {
            if (g_player->craftingOpen) {
                g_player->closeMenus();
                g_cursorCaptured = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                g_firstMouse = true;
            } else {
                g_player->closeMenus();
                g_player->craftingOpen = true;
                g_cursorCaptured = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                g_firstMouse = true;
            }
        } else if (g_player && g_player->craftingOpen && key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
            g_player->craftRecipe(key - GLFW_KEY_1);
        } else if (g_player && g_player->craftingOpen && key >= GLFW_KEY_A && key <= GLFW_KEY_D) {
            g_player->craftRecipe(9 + key - GLFW_KEY_A);
        }
    }
}

int main() {
#ifdef _WIN32
    // Ensure working directory is always where the executable is located
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        SetCurrentDirectoryA(exeDir.substr(0, lastSlash).c_str());
    }
#endif

    std::cout << "========================================================\n";
    std::cout << "  FantasyCraft: Isle of the Ancients (OpenGL C++ Voxel)\n";
    std::cout << "  Zespol Agentow: Architekt, Fizyka/Render, Swiat, Build\n";
    std::cout << "========================================================\n";

    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "[CRITICAL] Nie udalo sie zainicjalizowac GLFW!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    GLFWwindow* window = glfwCreateWindow(g_windowWidth, g_windowHeight, "FantasyCraft: Isle of the Ancients", nullptr, nullptr);
    if (!window) {
        std::cerr << "[CRITICAL] Nie udalo sie utworzyc okna GLFW!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync for smooth 60 FPS

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 2. Initialize GLAD
    int glVersion = gladLoadGL(glfwGetProcAddress);
    if (!glVersion) {
        std::cerr << "[CRITICAL] Nie udalo sie zaladowac funkcji OpenGL z GLAD!\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "[OpenGL] Wersja OpenGL: " << GLAD_VERSION_MAJOR(glVersion) << "." << GLAD_VERSION_MINOR(glVersion) << "\n";
    std::cout << "[OpenGL] GPU Renderer: " << glGetString(GL_RENDERER) << "\n";

    // 3. Configure Global OpenGL State
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_MULTISAMPLE);

    // 4. Load Shaders
    Shader voxelShader;
    if (!voxelShader.loadFromFiles("assets/shaders/voxel.vert", "assets/shaders/voxel.frag")) {
        std::cerr << "[CRITICAL] Blad ladowania shaderow voxelowych!\n";
        return -1;
    }

    // 5. Initialize Game Systems
    World world;
    world.init();
    g_world = &world;

    MobManager mobs;
    mobs.init(world.getTextureAtlas());
    g_mobs = &mobs;

    BlockParticles blockParticles;
    blockParticles.init(world.getTextureAtlas());

    HUD hud;
    hud.init();
    g_hud = &hud;

    // 6. Spawn Player
    int spawnX = 0;
    int spawnZ = 0;
    int spawnY = world.getSpawnHeight(spawnX, spawnZ) + 1;
    if (spawnY < SEA_LEVEL + 2) spawnY = SEA_LEVEL + 2;

    Player player(glm::vec3((float)spawnX + 0.5f, (float)spawnY, (float)spawnZ + 0.5f));
    player.isFlying = false; // Begin in normal survival movement; F toggles flight.
    g_player = &player;

    std::cout << "[Player] Pozycja startowa: (" << spawnX << ", " << spawnY << ", " << spawnZ << ")\n";
    std::cout << "[Player] Tryb chodzenia wlaczony (WASD, Spacja skok, F przelacza latanie)\n";

    // Initial world warm-up around player
    world.update(player.position);

    // Timing and FPS
    float lastFrameTime = (float)glfwGetTime();
    float fpsTimer = 0.0f;
    int frameCount = 0;
    float currentFPS = 60.0f;

    // OpenGL timer queries report GPU execution time without stalling the
    // current frame. The HUD derives an approximate busy percentage from it.
    constexpr int GPU_QUERY_COUNT = 3;
    GLuint gpuTimerQueries[GPU_QUERY_COUNT]{};
    bool gpuQueryPending[GPU_QUERY_COUNT]{};
    glGenQueries(GPU_QUERY_COUNT, gpuTimerQueries);
    int gpuQueryCursor = 0;
    float gpuFrameMs = 0.0f;

    // Direction of sunlight
    glm::vec3 sunDirection = glm::normalize(glm::vec3(0.5f, 0.85f, 0.35f));

    // 7. Main Game Loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = (float)glfwGetTime();
        float dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Cap dt to prevent physics exploding on freeze
        if (dt > 0.1f) dt = 0.1f;

        for (int i = 0; i < GPU_QUERY_COUNT; ++i) {
            if (!gpuQueryPending[i]) continue;
            GLint ready = GL_FALSE;
            glGetQueryObjectiv(gpuTimerQueries[i], GL_QUERY_RESULT_AVAILABLE, &ready);
            if (ready == GL_TRUE) {
                GLuint64 elapsedNs = 0;
                glGetQueryObjectui64v(gpuTimerQueries[i], GL_QUERY_RESULT, &elapsedNs);
                gpuFrameMs = static_cast<float>(elapsedNs) / 1000000.0f;
                gpuQueryPending[i] = false;
            }
        }
        int activeGpuQuery = -1;
        for (int offset = 0; offset < GPU_QUERY_COUNT; ++offset) {
            int candidate = (gpuQueryCursor + offset) % GPU_QUERY_COUNT;
            if (!gpuQueryPending[candidate]) {
                activeGpuQuery = candidate;
                break;
            }
        }

        // FPS Calculation
        frameCount++;
        fpsTimer += dt;
        if (fpsTimer >= 0.5f) {
            currentFPS = (float)frameCount / fpsTimer;
            frameCount = 0;
            fpsTimer = 0.0f;

            // Update Window Title with Live Diagnostics
            BiomeAtmosphere atmo = world.getAtmosphere(player.position);
            BlockId selBlock = player.getSelectedBlock();
            const BlockData& bData = BlockRegistry::get(selBlock);

            std::string title = "FantasyCraft: Isle of the Ancients | FPS: " + std::to_string((int)currentFPS) +
                                " | Pos: (" + std::to_string((int)player.position.x) + ", " +
                                std::to_string((int)player.position.y) + ", " +
                                std::to_string((int)player.position.z) + ") | Biome: " + atmo.displayName +
                                " | Blok: [" + bData.name + "]" +
                                " | HP: " + std::to_string((int)player.health) +
                                " | Moby: " + std::to_string(mobs.count()) +
                                (player.isFlying ? " [FLY MODE]" : " [WALK]");
            glfwSetWindowTitle(window, title.c_str());
        }

        // Handle Player Input & Physics
        if (g_cursorCaptured) {
            player.update(dt, world, window);
        }
        glm::ivec3 brokenPosition;
        BlockId brokenBlock;
        if (player.consumeBlockBreak(brokenPosition, brokenBlock))
            blockParticles.spawn(brokenPosition, brokenBlock);
        blockParticles.update(dt);

        // Update Chunk Streaming & Meshing
        world.update(player.position);
        mobs.update(dt, world, player);

        // Get atmospheric colors for current biome
        BiomeAtmosphere atmo = world.getAtmosphere(player.position);

        // Render Background (Clear with Biome Fog Color)
        glClearColor(atmo.fogColor.r, atmo.fogColor.g, atmo.fogColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute Matrices
        float aspect = (float)g_windowWidth / (float)(g_windowHeight > 0 ? g_windowHeight : 1);
        glm::mat4 view = player.camera.getViewMatrix();
        glm::mat4 proj = player.camera.getProjectionMatrix(aspect);

        // Setup Voxel Shader Uniforms
        voxelShader.use();
        voxelShader.setMat4("uView", view);
        voxelShader.setMat4("uProjection", proj);
        voxelShader.setVec3("uCameraPos", player.camera.position);
        voxelShader.setVec3("uSunDir", sunDirection);
        voxelShader.setVec3("uFogColor", atmo.fogColor);
        voxelShader.setFloat("uFogStart", atmo.fogStart);
        voxelShader.setFloat("uFogEnd", atmo.fogEnd);
        voxelShader.setFloat("uTime", currentFrameTime);
        voxelShader.setInt("uTextureAtlas", 0);

        // Render Voxel Chunks
        if (activeGpuQuery >= 0)
            glBeginQuery(GL_TIME_ELAPSED, gpuTimerQueries[activeGpuQuery]);
        world.render(voxelShader, player.position);
        mobs.render(voxelShader, world.getTextureAtlas(), player.position);
        world.getTextureAtlas().bind(0);
        blockParticles.render(voxelShader, player.position);

        // Render Block Target Outline
        RaycastResult target = player.getLookTarget(world);
        hud.renderBlockOutline(voxelShader, view, proj, target);

        // Render 2D HUD (Crosshair, Hotbar, Selected item)
        float gpuLoadPercent = dt > 0.0001f
            ? std::clamp(gpuFrameMs / (dt * 1000.0f) * 100.0f, 0.0f, 100.0f)
            : 0.0f;
        hud.render(g_windowWidth, g_windowHeight, player, world, currentFPS,
                   mobs.count(), gpuFrameMs, gpuLoadPercent);
        if (activeGpuQuery >= 0) {
            glEndQuery(GL_TIME_ELAPSED);
            gpuQueryPending[activeGpuQuery] = true;
            gpuQueryCursor = (activeGpuQuery + 1) % GPU_QUERY_COUNT;
        }

        // Swap Buffers and Poll Window Events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "[Shutdown] Zamykanie gry FantasyCraft...\n";

    glDeleteQueries(GPU_QUERY_COUNT, gpuTimerQueries);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
