#pragma once
#include "../engine/Common.h"
#include "../engine/Shader.h"
#include "../engine/TextureAtlas.h"
#include "../engine/Player.h"
#include "../world/World.h"

class HUD {
public:
    HUD();
    ~HUD();

    void init();
    void render(int windowWidth, int windowHeight, const Player& player, const World& world,
                float fps, int mobCount, float gpuFrameMs, float gpuLoadPercent);
    void handleClick(double mouseX, double mouseY, int button, Player& player, World& world,
                     int windowWidth, int windowHeight);
    void renderBlockOutline(const Shader& voxelShader, const glm::mat4& view, const glm::mat4& proj, const RaycastResult& lookTarget);

private:
    Shader m_uiShader;
    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;

    GLuint m_boxVAO = 0;
    GLuint m_boxVBO = 0;

    void initQuadMesh();
    void initBoxMesh();

    void drawQuad(float x, float y, float w, float h, const glm::vec4& color, int windowWidth, int windowHeight);
    void drawTexturedQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, const glm::vec4& color, int windowWidth, int windowHeight);
};
