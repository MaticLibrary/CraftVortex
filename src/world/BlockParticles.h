#pragma once

#include "Chunk.h"
#include "../engine/TextureAtlas.h"
#include "../engine/Shader.h"

class BlockParticles {
public:
    BlockParticles() = default;
    ~BlockParticles();

    BlockParticles(const BlockParticles&) = delete;
    BlockParticles& operator=(const BlockParticles&) = delete;

    void init(const TextureAtlas& atlas);
    void spawn(const glm::ivec3& blockPosition, BlockId blockId);
    void update(float dt);
    void render(const Shader& shader, const glm::vec3& playerPosition);

private:
    struct Particle {
        glm::vec3 position{0.0f};
        glm::vec3 velocity{0.0f};
        BlockId block = BLOCK_STONE;
        float age = 0.0f;
        float lifetime = 0.7f;
        float size = 0.16f;
    };

    struct Mesh {
        GLuint vao = 0;
        GLuint vbo = 0;
    };

    std::vector<Particle> m_particles;
    Mesh m_blockMeshes[BLOCK_COUNT]{};
    uint32_t m_randomState = 0x9e3779b9u;

    float random01();
};
