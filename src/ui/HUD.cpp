#include "HUD.h"
#include <vector>

HUD::HUD() {}

HUD::~HUD() {
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        glDeleteBuffers(1, &m_quadVBO);
    }
    if (m_boxVAO != 0) {
        glDeleteVertexArrays(1, &m_boxVAO);
        glDeleteBuffers(1, &m_boxVBO);
    }
}

void HUD::init() {
    bool ok = m_uiShader.loadFromFiles("assets/shaders/ui.vert", "assets/shaders/ui.frag");
    if (!ok) {
        std::cerr << "[HUD] Failed to load UI shaders!\n";
    }

    initQuadMesh();
    initBoxMesh();
}

void HUD::initQuadMesh() {
    // Dynamic quad buffer for 2D rendering
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);

    // Vertex structure: x, y, u, v, r, g, b, a
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 6, nullptr, GL_DYNAMIC_DRAW);

    // Pos (x, y)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // UV (u, v)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));

    // Color (r, g, b, a)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));

    glBindVertexArray(0);
}

void HUD::initBoxMesh() {
    // 12 edges of a unit cube for the block selection outline
    float lines[] = {
        // Bottom square
        0, 0, 0,  1, 0, 0,
        1, 0, 0,  1, 0, 1,
        1, 0, 1,  0, 0, 1,
        0, 0, 1,  0, 0, 0,
        // Top square
        0, 1, 0,  1, 1, 0,
        1, 1, 0,  1, 1, 1,
        1, 1, 1,  0, 1, 1,
        0, 1, 1,  0, 1, 0,
        // Vertical pillars
        0, 0, 0,  0, 1, 0,
        1, 0, 0,  1, 1, 0,
        1, 0, 1,  1, 1, 1,
        0, 0, 1,  0, 1, 1
    };

    glGenVertexArrays(1, &m_boxVAO);
    glGenBuffers(1, &m_boxVBO);

    glBindVertexArray(m_boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void HUD::drawQuad(float x, float y, float w, float h, const glm::vec4& color, int windowWidth, int windowHeight) {
    float verts[] = {
        // x, y, u, v, r, g, b, a
        x,     y,     0.0f, 0.0f, color.r, color.g, color.b, color.a,
        x + w, y,     1.0f, 0.0f, color.r, color.g, color.b, color.a,
        x + w, y + h, 1.0f, 1.0f, color.r, color.g, color.b, color.a,

        x,     y,     0.0f, 0.0f, color.r, color.g, color.b, color.a,
        x + w, y + h, 1.0f, 1.0f, color.r, color.g, color.b, color.a,
        x,     y + h, 0.0f, 1.0f, color.r, color.g, color.b, color.a
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    m_uiShader.setBool("uUseTexture", false);
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void HUD::drawTexturedQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, const glm::vec4& color, int windowWidth, int windowHeight) {
    float verts[] = {
        x,     y,     u0, v0, color.r, color.g, color.b, color.a,
        x + w, y,     u1, v0, color.r, color.g, color.b, color.a,
        x + w, y + h, u1, v1, color.r, color.g, color.b, color.a,

        x,     y,     u0, v0, color.r, color.g, color.b, color.a,
        x + w, y + h, u1, v1, color.r, color.g, color.b, color.a,
        x,     y + h, u0, v1, color.r, color.g, color.b, color.a
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    m_uiShader.setBool("uUseTexture", true);
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void HUD::renderBlockOutline(const Shader& voxelShader, const glm::mat4& view, const glm::mat4& proj, const RaycastResult& lookTarget) {
    if (!lookTarget.hit) return;

    // Slightly expand the outline box to prevent Z-fighting with block faces
    glm::vec3 pos = glm::vec3(lookTarget.blockPos) - glm::vec3(0.002f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
    model = glm::scale(model, glm::vec3(1.004f));

    voxelShader.use();
    voxelShader.setMat4("uModel", model);
    voxelShader.setMat4("uView", view);
    voxelShader.setMat4("uProjection", proj);

    glLineWidth(2.5f);
    glBindVertexArray(m_boxVAO);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}

void HUD::render(int windowWidth, int windowHeight, const Player& player, const World& world,
                 float fps, int mobCount, float gpuFrameMs, float gpuLoadPercent) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    // The screen-space quads use top-left coordinates with a downward Y axis,
    // so their winding is reversed from the world's front faces.
    glDisable(GL_CULL_FACE);

    m_uiShader.use();
    glm::mat4 ortho = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
    m_uiShader.setMat4("uOrtho", ortho);

    // 1. Crosshair in center of screen
    float cx = (float)windowWidth * 0.5f;
    float cy = (float)windowHeight * 0.5f;
    glm::vec4 crossColor = player.getHitConfirm() > 0.0f
        ? glm::vec4(1.0f, 0.78f, 0.2f, 1.0f)
        : glm::vec4(1.0f, 1.0f, 1.0f, 0.85f);
    glm::vec4 shadowColor(0.0f, 0.0f, 0.0f, 0.6f);

    // Horizontal bar
    drawQuad(cx - 8, cy - 2, 16, 4, shadowColor, windowWidth, windowHeight);
    drawQuad(cx - 7, cy - 1, 14, 2, crossColor, windowWidth, windowHeight);

    // Vertical bar
    drawQuad(cx - 2, cy - 8, 4, 16, shadowColor, windowWidth, windowHeight);
    drawQuad(cx - 1, cy - 7, 2, 14, crossColor, windowWidth, windowHeight);

    if (player.getMiningProgress() > 0.0f) {
        drawQuad(cx - 42, cy + 18, 84, 7, glm::vec4(0.03f, 0.03f, 0.04f, 0.85f), windowWidth, windowHeight);
        drawQuad(cx - 40, cy + 20, 80.0f * player.getMiningProgress(), 3,
                 glm::vec4(1.0f, 0.72f, 0.18f, 0.95f), windowWidth, windowHeight);
    }

    // 2. Hotbar at bottom center
    int numSlots = Player::HOTBAR_SLOTS;
    float slotSize = 42.0f;
    float gap = 4.0f;
    float totalW = numSlots * slotSize + (numSlots - 1) * gap;
    float startX = (windowWidth - totalW) * 0.5f;
    float startY = (float)windowHeight - slotSize - 18.0f;

    // Hotbar background banner
    drawQuad(startX - 6, startY - 6, totalW + 12, slotSize + 12, glm::vec4(0.05f, 0.05f, 0.08f, 0.65f), windowWidth, windowHeight);

    world.getTextureAtlas().bind(0);
    m_uiShader.setInt("uTexture", 0);

    for (int i = 0; i < numSlots; ++i) {
        float sx = startX + i * (slotSize + gap);
        float sy = startY;

        bool isSelected = (i == player.selectedSlot);

        // Slot background
        glm::vec4 slotBg = isSelected ? glm::vec4(0.25f, 0.28f, 0.35f, 0.85f) : glm::vec4(0.12f, 0.12f, 0.16f, 0.8f);
        drawQuad(sx, sy, slotSize, slotSize, slotBg, windowWidth, windowHeight);

        // Draw block texture icon inside slot
        BlockId bId = player.hotbar[i];
        const BlockData& bData = BlockRegistry::get(bId);
        float u0, v0, u1, v1;
        world.getTextureAtlas().getTileUVs(bData.textures.top, u0, v0, u1, v1);

        drawTexturedQuad(sx + 4, sy + 4, slotSize - 8, slotSize - 8, u0, v0, u1, v1, glm::vec4(1.0f), windowWidth, windowHeight);

        // Small count meter under each slot (creative flight keeps blocks unlimited).
        int count = player.isFlying ? 64 : player.inventory[bId];
        drawQuad(sx + 3, sy + slotSize + 2, slotSize - 6, 3,
                 glm::vec4(0.02f, 0.02f, 0.03f, 0.8f), windowWidth, windowHeight);
        if (count > 0) {
            float fill = (slotSize - 6) * std::min(1.0f, count / 64.0f);
            drawQuad(sx + 3, sy + slotSize + 2, fill, 3,
                     glm::vec4(0.35f, 0.85f, 0.5f, 0.9f), windowWidth, windowHeight);
        }
        // Selected slot glowing border
        if (isSelected) {
            float th = 3.0f;
            glm::vec4 goldGlow(1.0f, 0.85f, 0.2f, 0.95f);
            drawQuad(sx - th, sy - th, slotSize + 2 * th, th, goldGlow, windowWidth, windowHeight); // top
            drawQuad(sx - th, sy + slotSize, slotSize + 2 * th, th, goldGlow, windowWidth, windowHeight); // bottom
            drawQuad(sx - th, sy, th, slotSize, goldGlow, windowWidth, windowHeight); // left
            drawQuad(sx + slotSize, sy, th, slotSize, goldGlow, windowWidth, windowHeight); // right
        } else {
            // Subtle normal border
            float th = 1.0f;
            glm::vec4 subtleBorder(0.35f, 0.35f, 0.4f, 0.5f);
            drawQuad(sx, sy, slotSize, th, subtleBorder, windowWidth, windowHeight);
            drawQuad(sx, sy + slotSize - th, slotSize, th, subtleBorder, windowWidth, windowHeight);
            drawQuad(sx, sy, th, slotSize, subtleBorder, windowWidth, windowHeight);
            drawQuad(sx + slotSize - th, sy, th, slotSize, subtleBorder, windowWidth, windowHeight);
        }
    }

    // Compact seven-segment digits are enough to mark the four recipe keys.
    auto drawDigit = [&](int digit, float x, float y, float scale, const glm::vec4& color) {
        static const int segments[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
        if (digit < 0 || digit > 9) return;
        int mask = segments[digit];
        float t = scale;
        if (mask & 0x01) drawQuad(x + t, y, 3*t, t, color, windowWidth, windowHeight);
        if (mask & 0x02) drawQuad(x + 4*t, y + t, t, 3*t, color, windowWidth, windowHeight);
        if (mask & 0x04) drawQuad(x + 4*t, y + 5*t, t, 3*t, color, windowWidth, windowHeight);
        if (mask & 0x08) drawQuad(x + t, y + 8*t, 3*t, t, color, windowWidth, windowHeight);
        if (mask & 0x10) drawQuad(x, y + 5*t, t, 3*t, color, windowWidth, windowHeight);
        if (mask & 0x20) drawQuad(x, y + t, t, 3*t, color, windowWidth, windowHeight);
        if (mask & 0x40) drawQuad(x + t, y + 4*t, 3*t, t, color, windowWidth, windowHeight);
    };

    auto drawTinyText = [&](const std::string& text, float x, float y, float pixel, const glm::vec4& color) {
        auto glyph = [](char ch) -> std::array<int, 5> {
            switch (ch) {
                case 'A': return {2,5,7,5,5}; case 'B': return {6,5,6,5,6}; case 'C': return {3,4,4,4,3};
                case 'D': return {6,5,5,5,6}; case 'E': return {7,4,6,4,7}; case 'F': return {7,4,6,4,4};
                case 'G': return {3,4,5,5,3}; case 'H': return {5,5,7,5,5}; case 'I': return {7,2,2,2,7};
                case 'J': return {1,1,1,5,2}; case 'K': return {5,5,6,5,5}; case 'L': return {4,4,4,4,7};
                case 'M': return {5,7,7,5,5}; case 'N': return {5,7,7,7,5}; case 'O': return {2,5,5,5,2};
                case 'P': return {6,5,6,4,4}; case 'Q': return {2,5,5,3,1}; case 'R': return {6,5,6,5,5};
                case 'S': return {3,4,2,1,6}; case 'T': return {7,2,2,2,2}; case 'U': return {5,5,5,5,7};
                case 'V': return {5,5,5,5,2}; case 'W': return {5,5,7,7,5}; case 'X': return {5,5,2,5,5};
                case 'Y': return {5,5,2,2,2}; case 'Z': return {7,1,2,4,7};
                case '0': return {7,5,5,5,7}; case '1': return {2,6,2,2,7}; case '2': return {6,1,2,4,7};
                case '3': return {6,1,2,1,6}; case '4': return {5,5,7,1,1}; case '5': return {7,4,6,1,6};
                case '6': return {3,4,7,5,7}; case '7': return {7,1,2,2,2}; case '8': return {7,5,7,5,7};
                case '9': return {7,5,7,1,6}; case '+': return {0,2,7,2,0}; case '>': return {4,2,1,2,4};
                case '-': return {0,0,7,0,0}; case '%': return {5,1,2,4,5};
                default: return {0,0,0,0,0};
            }
        };
        float cursor = x;
        for (char ch : text) {
            auto rows = glyph(ch);
            for (int row = 0; row < 5; ++row) {
                for (int col = 0; col < 3; ++col) {
                    if (rows[row] & (1 << (2 - col)))
                        drawQuad(cursor + col * pixel, y + row * pixel, pixel, pixel, color, windowWidth, windowHeight);
                }
            }
            cursor += pixel * 4.0f;
        }
    };

    for (int i = 0; i < numSlots; ++i) {
        float sx = startX + i * (slotSize + gap);
        int count = player.isFlying ? 64 : player.inventory[player.hotbar[i]];
        drawTinyText(std::to_string(count), sx + 25.0f, startY + 30.0f, 1.15f,
                     glm::vec4(0.05f, 0.05f, 0.07f, 0.9f));
        drawTinyText(std::to_string(count), sx + 24.0f, startY + 29.0f, 1.15f,
                     glm::vec4(1.0f, 1.0f, 0.95f, 1.0f));
    }

    drawTinyText("HOTBAR", startX + 4, startY - 15, 1.0f, glm::vec4(0.95f, 0.92f, 0.76f, 0.95f));
    static const char* blockLabels[BLOCK_COUNT] = {
        "AIR", "GRASS", "DIRT", "STONE", "COBBLE", "RUNE", "AMETHYST", "MANA",
        "GOLD", "SANDSTONE", "HIEROGLYPH", "DARKWOOD", "LEAVES", "PLANKS", "BRICK",
        "LANTERN", "WATER", "SAND", "OBSIDIAN", "MOSS", "SNOW", "MUD", "BASALT", "MEADOW",
        "BOOKS", "CHEST", "CHAIN", "MARBLE", "GOLDTRIM", "CRYSTAL"
    };
    BlockId selectedBlock = player.getSelectedBlock();
    int selectedCount = player.isFlying ? 64 : player.inventory[selectedBlock];
    std::string selectedLabel = std::string(blockLabels[selectedBlock]) + " X " + std::to_string(selectedCount);
    drawTinyText(selectedLabel, cx - selectedLabel.size() * 4.0f, startY - 15.0f, 1.0f,
                 glm::vec4(1.0f, 0.9f, 0.64f, 0.98f));

    drawQuad((float)windowWidth - 122.0f, 12.0f, 110.0f, 150.0f,
             glm::vec4(0.05f, 0.06f, 0.09f, 0.72f), windowWidth, windowHeight);
    glm::vec4 hintColor(0.88f, 0.91f, 0.96f, 0.95f);
    drawTinyText("FPS " + std::to_string((int)fps), (float)windowWidth - 114.0f, 16.0f, 1.0f,
                 glm::vec4(0.62f, 1.0f, 0.7f, 1.0f));
    drawTinyText("GPU " + std::to_string((int)std::round(gpuFrameMs)) + "MS",
                 (float)windowWidth - 114.0f, 25.0f, 1.0f, glm::vec4(0.65f, 0.85f, 1.0f, 1.0f));
    drawTinyText("GPU EST " + std::to_string((int)std::round(gpuLoadPercent)) + "%",
                 (float)windowWidth - 114.0f, 34.0f, 1.0f, glm::vec4(0.72f, 0.8f, 1.0f, 1.0f));
    drawTinyText("MOBS " + std::to_string(mobCount), (float)windowWidth - 114.0f, 43.0f, 1.0f, hintColor);
    drawQuad((float)windowWidth - 116.0f, 54.0f, 98.0f, 1.0f,
             glm::vec4(0.55f, 0.61f, 0.72f, 0.7f), windowWidth, windowHeight);
    drawTinyText("WASD MOVE", (float)windowWidth - 114.0f, 60.0f, 1.0f, hintColor);
    drawTinyText("SPACE JUMP", (float)windowWidth - 114.0f, 69.0f, 1.0f, hintColor);
    drawTinyText("LMB MINE", (float)windowWidth - 114.0f, 78.0f, 1.0f, hintColor);
    drawTinyText("RMB BUILD CHEST", (float)windowWidth - 114.0f, 87.0f, 1.0f, hintColor);
    drawTinyText("Q CHANGE TOOL", (float)windowWidth - 114.0f, 96.0f, 1.0f, hintColor);
    drawTinyText("E INVENTORY", (float)windowWidth - 114.0f, 105.0f, 1.0f, hintColor);
    drawTinyText("C CRAFT F FLY", (float)windowWidth - 114.0f, 114.0f, 1.0f, hintColor);
    drawTinyText("WHEEL ITEMS", (float)windowWidth - 114.0f, 123.0f, 1.0f, hintColor);

    // 3. Top-left Status bar backdrop
    drawQuad(12, 12, 220, 70, glm::vec4(0.08f, 0.08f, 0.12f, 0.78f), windowWidth, windowHeight);
    drawTinyText("HP", 22.0f, 15.0f, 1.0f, glm::vec4(1.0f, 0.78f, 0.8f, 1.0f));
    drawTinyText(std::to_string((int)std::ceil(player.health)), 44.0f, 15.0f, 1.0f,
                 glm::vec4(1.0f, 0.9f, 0.9f, 1.0f));
    drawQuad(22, 23, 184, 16, glm::vec4(0.12f, 0.04f, 0.05f, 0.95f), windowWidth, windowHeight);
    if (player.health > 0.0f)
        drawQuad(24, 25, 180.0f * (player.health / 100.0f), 12,
                 glm::vec4(0.78f, 0.12f, 0.19f, 0.95f), windowWidth, windowHeight);
    drawTinyText("TOOL", 22.0f, 45.0f, 1.0f, glm::vec4(0.86f, 0.9f, 0.98f, 1.0f));
    drawTinyText(player.getToolName(), 48.0f, 45.0f, 1.0f, glm::vec4(1.0f, 0.84f, 0.38f, 1.0f));
    drawTinyText("MODE", 133.0f, 45.0f, 1.0f, glm::vec4(0.86f, 0.9f, 0.98f, 1.0f));
    drawTinyText(player.isFlying ? "FLIGHT" : "WALK", 133.0f, 55.0f, 1.0f,
                 player.isFlying ? glm::vec4(0.5f, 0.85f, 1.0f, 1.0f) : glm::vec4(0.55f, 1.0f, 0.65f, 1.0f));

    if (player.getDamageFlash() > 0.0f) {
        float alpha = std::min(0.45f, player.getDamageFlash() * 1.4f);
        glm::vec4 hurt(0.9f, 0.025f, 0.04f, alpha);
        drawQuad(0, 0, (float)windowWidth, 7, hurt, windowWidth, windowHeight);
        drawQuad(0, (float)windowHeight - 7, (float)windowWidth, 7, hurt, windowWidth, windowHeight);
        drawQuad(0, 0, 7, (float)windowHeight, hurt, windowWidth, windowHeight);
        drawQuad((float)windowWidth - 7, 0, 7, (float)windowHeight, hurt, windowWidth, windowHeight);
    }

    drawTinyText("ARROWS " + std::to_string(player.arrows), 22.0f, 68.0f, 1.0f,
                 glm::vec4(0.9f, 0.82f, 0.58f, 1.0f));

    if (player.craftingOpen || player.inventoryOpen || player.chestOpen) {
        drawQuad(0, 0, (float)windowWidth, (float)windowHeight,
                 glm::vec4(0.015f, 0.02f, 0.035f, 0.68f), windowWidth, windowHeight);

        if (player.craftingOpen) {
            float panelW = 830.0f, panelH = 600.0f;
            float panelX = cx - panelW * 0.5f, panelY = cy - panelH * 0.5f;
            drawQuad(panelX, panelY, panelW, panelH, glm::vec4(0.035f, 0.045f, 0.07f, 0.98f), windowWidth, windowHeight);
            drawQuad(panelX + 18, panelY + 18, panelW - 36, 4, glm::vec4(0.7f, 0.8f, 1.0f, 0.9f), windowWidth, windowHeight);
            drawTinyText("CRAFTING  CLICK OR KEYS 1-9 A-D", panelX + 26, panelY + 30, 1.4f,
                         glm::vec4(0.9f, 0.94f, 1.0f, 1.0f));
            world.getTextureAtlas().bind(0);
            m_uiShader.setInt("uTexture", 0);

            static const char* labels[Player::CRAFT_RECIPE_COUNT] = {
                "1 WOOD > 4 PLANK", "2 STONE + AMETHYST > RUNE", "2 SAND + STONE > SANDSTONE",
                "2 BASALT + OBSIDIAN > LANTERN", "3 PLANK > BOOKSHELF", "4 PLANK + GOLD > CHEST",
                "STONE + GOLD > 3 CHAIN", "4 STONE > 4 MARBLE", "RUNE + AMETHYST > 2 CRYSTAL",
                "2 PLANK + STONE > AXE", "PLANK + 2 STONE > SWORD", "3 PLANK + AMETHYST > BOW",
                "PLANK + STONE > 8 ARROWS"
            };
            auto drawBlockIcon = [&](BlockId id, float x, float y, float size) {
                if (id == BLOCK_AIR) return;
                const BlockData& data = BlockRegistry::get(id);
                float iu0, iv0, iu1, iv1;
                world.getTextureAtlas().getTileUVs(data.textures.top, iu0, iv0, iu1, iv1);
                drawTexturedQuad(x, y, size, size, iu0, iv0, iu1, iv1, glm::vec4(1.0f), windowWidth, windowHeight);
            };

            for (int i = 0; i < Player::CRAFT_RECIPE_COUNT; ++i) {
                int col = i % 2, row = i / 2;
                float cardX = panelX + 18.0f + col * 404.0f;
                float cardY = panelY + 66.0f + row * 72.0f;
                CraftRecipe recipe = Player::getCraftRecipe(i);
                bool canCraft = player.canCraftRecipe(i);
                glm::vec4 cardColor = canCraft ? glm::vec4(0.14f, 0.29f, 0.20f, 0.96f)
                                               : glm::vec4(0.105f, 0.12f, 0.17f, 0.94f);
                drawQuad(cardX, cardY, 392.0f, 66.0f, cardColor, windowWidth, windowHeight);
                if (i < 9) drawDigit(i + 1, cardX + 9, cardY + 22, 1.5f, glm::vec4(1.0f, 0.84f, 0.34f, 1.0f));
                else drawTinyText(std::string(1, static_cast<char>('A' + i - 9)), cardX + 9, cardY + 24, 1.4f,
                                  glm::vec4(1.0f, 0.84f, 0.34f, 1.0f));
                drawBlockIcon(recipe.inputA, cardX + 34, cardY + 15, 32);
                if (recipe.amountB > 0) {
                    drawQuad(cardX + 72, cardY + 30, 8, 2, glm::vec4(0.92f), windowWidth, windowHeight);
                    drawQuad(cardX + 75, cardY + 27, 2, 8, glm::vec4(0.92f), windowWidth, windowHeight);
                    drawBlockIcon(recipe.inputB, cardX + 88, cardY + 15, 32);
                }
                drawQuad(cardX + 129, cardY + 31, 20, 2, glm::vec4(0.9f, 0.9f, 0.95f, 0.95f), windowWidth, windowHeight);
                drawQuad(cardX + 143, cardY + 26, 6, 12, glm::vec4(0.9f, 0.9f, 0.95f, 0.95f), windowWidth, windowHeight);
                if (recipe.outputType == CraftOutputType::Block) {
                    drawBlockIcon(recipe.output, cardX + 158, cardY + 15, 32);
                } else if (recipe.outputType == CraftOutputType::Tool && recipe.toolOutput == ToolType::Bow) {
                    glm::vec4 bowColor(0.75f, 0.48f, 0.2f, 1.0f);
                    drawQuad(cardX + 166, cardY + 18, 3, 26, bowColor, windowWidth, windowHeight);
                    drawQuad(cardX + 166, cardY + 18, 10, 3, bowColor, windowWidth, windowHeight);
                    drawQuad(cardX + 166, cardY + 41, 10, 3, bowColor, windowWidth, windowHeight);
                    drawQuad(cardX + 176, cardY + 20, 1, 22, glm::vec4(0.9f), windowWidth, windowHeight);
                } else if (recipe.outputType == CraftOutputType::Arrows) {
                    drawQuad(cardX + 158, cardY + 30, 28, 2, glm::vec4(0.75f, 0.58f, 0.34f, 1.0f), windowWidth, windowHeight);
                    drawQuad(cardX + 181, cardY + 25, 3, 12, glm::vec4(0.75f), windowWidth, windowHeight);
                } else {
                    glm::vec4 metal(0.7f, 0.76f, 0.84f, 1.0f);
                    drawQuad(cardX + 160, cardY + 29, 28, 4, metal, windowWidth, windowHeight);
                    drawQuad(cardX + 160, cardY + 23, 4, 16, metal, windowWidth, windowHeight);
                }
                drawTinyText(labels[i], cardX + 202, cardY + 27, 1.0f,
                             canCraft ? glm::vec4(0.94f, 0.97f, 0.94f, 1.0f)
                                      : glm::vec4(0.62f, 0.66f, 0.73f, 0.95f));
            }
            drawTinyText("A-D = AXE / SWORD / BOW / ARROWS     ESC CLOSE", panelX + 26, panelY + 574, 1.0f,
                         glm::vec4(0.65f, 0.73f, 0.86f, 1.0f));
        } else {
            const bool showChest = player.chestOpen;
            float panelW = showChest ? 790.0f : 460.0f;
            float panelH = 520.0f;
            float panelX = cx - panelW * 0.5f, panelY = cy - panelH * 0.5f;
            drawQuad(panelX, panelY, panelW, panelH, glm::vec4(0.035f, 0.045f, 0.07f, 0.98f), windowWidth, windowHeight);
            drawQuad(panelX + 18, panelY + 18, panelW - 36, 4, glm::vec4(0.7f, 0.8f, 1.0f, 0.9f), windowWidth, windowHeight);
            drawTinyText(showChest ? "CHEST   LMB STACK  RMB ONE" : "INVENTORY   CLICK ITEM TO HOTBAR",
                         panelX + 26, panelY + 30, 1.25f, glm::vec4(0.9f, 0.94f, 1.0f, 1.0f));
            float gridY = panelY + 78.0f;
            float playerGridX = showChest ? panelX + 34.0f : panelX + 94.0f;
            float chestGridX = panelX + 468.0f;
            drawTinyText("PLAYER ITEMS", playerGridX, panelY + 60.0f, 1.0f, glm::vec4(0.7f, 0.85f, 1.0f, 1.0f));
            if (showChest) drawTinyText("TREASURE", chestGridX, panelY + 60.0f, 1.0f, glm::vec4(1.0f, 0.83f, 0.45f, 1.0f));

            world.getTextureAtlas().bind(0);
            m_uiShader.setInt("uTexture", 0);
            auto drawGrid = [&](const std::array<int, BLOCK_COUNT>& counts, float gridX) {
                for (int id = 0; id < BLOCK_COUNT; ++id) {
                    int col = id % 5, row = id / 5;
                    float sx = gridX + col * 56.0f, sy = gridY + row * 56.0f;
                    bool selected = id > 0 && player.hotbar[player.selectedSlot] == id;
                    drawQuad(sx, sy, 48, 48,
                             selected ? glm::vec4(0.27f, 0.25f, 0.17f, 0.95f) : glm::vec4(0.12f, 0.14f, 0.19f, 0.95f),
                             windowWidth, windowHeight);
                    if (id > 0) {
                        const BlockData& data = BlockRegistry::get(static_cast<BlockId>(id));
                        float iu0, iv0, iu1, iv1;
                        world.getTextureAtlas().getTileUVs(data.textures.top, iu0, iv0, iu1, iv1);
                        drawTexturedQuad(sx + 5, sy + 4, 38, 38, iu0, iv0, iu1, iv1,
                                         glm::vec4(1.0f), windowWidth, windowHeight);
                        if (counts[id] > 0) {
                            std::string amount = std::to_string(counts[id]);
                            drawTinyText(amount, sx + 43.0f - amount.size() * 4.0f, sy + 40.0f, 1.0f,
                                         glm::vec4(1.0f, 0.96f, 0.8f, 1.0f));
                        }
                        if (selected) {
                            drawQuad(sx, sy, 48, 2, glm::vec4(1.0f, 0.82f, 0.25f, 1.0f), windowWidth, windowHeight);
                            drawQuad(sx, sy + 46, 48, 2, glm::vec4(1.0f, 0.82f, 0.25f, 1.0f), windowWidth, windowHeight);
                        }
                    }
                }
            };
            drawGrid(player.inventory, playerGridX);
            const ChestInventory* chest = showChest ? world.getChestInventory(player.openChestPosition) : nullptr;
            if (chest) drawGrid(chest->blocks, chestGridX);

            float arrowY = panelY + 422.0f;
            auto drawArrowSlot = [&](float x, int count) {
                drawQuad(x, arrowY, 112, 30, glm::vec4(0.13f, 0.15f, 0.2f, 0.95f), windowWidth, windowHeight);
                drawQuad(x + 8, arrowY + 14, 21, 2, glm::vec4(0.75f, 0.58f, 0.34f, 1.0f), windowWidth, windowHeight);
                drawQuad(x + 26, arrowY + 9, 3, 12, glm::vec4(0.78f, 0.82f, 0.9f, 1.0f), windowWidth, windowHeight);
                drawTinyText("ARROWS " + std::to_string(count), x + 36, arrowY + 11, 1.0f,
                             glm::vec4(0.95f, 0.9f, 0.74f, 1.0f));
            };
            drawArrowSlot(playerGridX, player.arrows);
            if (showChest && chest) drawArrowSlot(chestGridX, chest->arrows);

            float toolX = panelX + panelW * 0.5f - 198.0f;
            float toolY = panelY + 466.0f;
            static const char* toolLabels[4] = {"PICKAXE", "AXE", "SWORD", "BOW"};
            for (int t = 0; t < 4; ++t) {
                float bx = toolX + t * 100.0f;
                bool owned = player.ownedTools[t];
                bool equipped = static_cast<int>(player.selectedTool) == t;
                glm::vec4 button = !owned ? glm::vec4(0.09f, 0.095f, 0.12f, 0.9f) :
                                   (equipped ? glm::vec4(0.38f, 0.29f, 0.12f, 0.98f) : glm::vec4(0.14f, 0.18f, 0.23f, 0.98f));
                drawQuad(bx, toolY, 94, 32, button, windowWidth, windowHeight);
                drawTinyText(toolLabels[t], bx + 7, toolY + 13, 1.0f,
                             owned ? glm::vec4(0.96f, 0.92f, 0.8f, 1.0f) : glm::vec4(0.42f, 0.44f, 0.48f, 1.0f));
            }
        }
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void HUD::handleClick(double mouseX, double mouseY, int button, Player& player, World& world,
                     int windowWidth, int windowHeight) {
    if (button != GLFW_MOUSE_BUTTON_LEFT && button != GLFW_MOUSE_BUTTON_RIGHT) return;
    float cx = windowWidth * 0.5f, cy = windowHeight * 0.5f;

    if (player.craftingOpen) {
        constexpr float panelW = 830.0f, panelH = 600.0f;
        float panelX = cx - panelW * 0.5f, panelY = cy - panelH * 0.5f;
        for (int i = 0; i < Player::CRAFT_RECIPE_COUNT; ++i) {
            int col = i % 2, row = i / 2;
            float cardX = panelX + 18.0f + col * 404.0f;
            float cardY = panelY + 66.0f + row * 72.0f;
            if (mouseX >= cardX && mouseX <= cardX + 392.0f &&
                mouseY >= cardY && mouseY <= cardY + 66.0f) {
                player.craftRecipe(i);
                return;
            }
        }
        return;
    }
    if (!player.inventoryOpen && !player.chestOpen) return;

    const bool showChest = player.chestOpen;
    float panelW = showChest ? 790.0f : 460.0f;
    float panelH = 520.0f;
    float panelX = cx - panelW * 0.5f, panelY = cy - panelH * 0.5f;
    float gridY = panelY + 78.0f;
    float playerGridX = showChest ? panelX + 34.0f : panelX + 94.0f;
    float chestGridX = panelX + 468.0f;
    const int clickAmount = button == GLFW_MOUSE_BUTTON_LEFT ? 999 : 1;

    auto slotAt = [&](float gridX, double x, double y) -> int {
        float localX = static_cast<float>(x) - gridX;
        float localY = static_cast<float>(y) - gridY;
        if (localX < 0 || localY < 0) return -1;
        int col = static_cast<int>(localX / 56.0f);
        int row = static_cast<int>(localY / 56.0f);
        if (col < 0 || col >= 5 || row < 0 || row >= 6) return -1;
        if (localX - col * 56.0f > 48.0f || localY - row * 56.0f > 48.0f) return -1;
        return row * 5 + col;
    };

    auto transfer = [&](int& from, int& to) {
        int amount = std::min(clickAmount, from);
        amount = std::min(amount, 999 - to);
        if (amount <= 0) return;
        from -= amount;
        to += amount;
    };

    int playerSlot = slotAt(playerGridX, mouseX, mouseY);
    int chestSlot = showChest ? slotAt(chestGridX, mouseX, mouseY) : -1;
    ChestInventory* chest = showChest ? world.getChestInventory(player.openChestPosition) : nullptr;
    if (playerSlot > 0 && playerSlot < BLOCK_COUNT) {
        BlockId id = static_cast<BlockId>(playerSlot);
        if (showChest && chest) transfer(player.inventory[id], chest->blocks[id]);
        else if (player.inventory[id] > 0) player.hotbar[player.selectedSlot] = id;
        return;
    }
    if (chestSlot > 0 && chestSlot < BLOCK_COUNT && chest) {
        BlockId id = static_cast<BlockId>(chestSlot);
        transfer(chest->blocks[id], player.inventory[id]);
        return;
    }

    float arrowY = panelY + 422.0f;
    auto inArrowSlot = [&](float x) {
        return mouseX >= x && mouseX <= x + 112.0f && mouseY >= arrowY && mouseY <= arrowY + 30.0f;
    };
    if (showChest && chest && inArrowSlot(playerGridX)) {
        int amount = std::min(clickAmount, player.arrows);
        amount = std::min(amount, 999 - chest->arrows);
        player.arrows -= amount;
        chest->arrows += amount;
        return;
    }
    if (showChest && chest && inArrowSlot(chestGridX)) {
        int amount = std::min(clickAmount, chest->arrows);
        amount = std::min(amount, 999 - player.arrows);
        chest->arrows -= amount;
        player.arrows += amount;
        return;
    }

    float toolX = panelX + panelW * 0.5f - 198.0f;
    float toolY = panelY + 466.0f;
    if (mouseY >= toolY && mouseY <= toolY + 32.0f) {
        int toolIndex = static_cast<int>((mouseX - toolX) / 100.0f);
        if (toolIndex >= 0 && toolIndex < 4 && mouseX <= toolX + toolIndex * 100.0f + 94.0f &&
            player.ownedTools[toolIndex])
            player.selectedTool = static_cast<ToolType>(toolIndex);
    }
}
