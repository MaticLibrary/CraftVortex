#pragma once
#include "Chunk.h"
#include "World.h"
#include "../engine/Player.h"

enum class MobKind : uint8_t { Elf, Orc, Human, Fairy };

class MobManager {
public:
    MobManager() = default;
    ~MobManager();

    void init(const TextureAtlas& atlas);
    void update(float dt, const World& world, Player& player);
    void render(const Shader& shader, const TextureAtlas& atlas, const glm::vec3& playerPos);
    bool tryHit(const glm::vec3& origin, const glm::vec3& direction, float damage = 16.0f,
                float maxRange = 6.0f, const World* world = nullptr);
    int count() const {
        int alive = 0;
        for (const auto& entry : m_mobs) if (entry.second.health > 0.0f) ++alive;
        return alive;
    }

private:
    struct Mob {
        MobKind kind = MobKind::Human;
        glm::vec3 position{0.0f};
        glm::vec3 wanderTarget{0.0f};
        float health = 30.0f;
        float attackTimer = 0.0f;
        float wanderTimer = 0.0f;
        uint32_t randomState = 1;
    };
    struct Mesh {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLsizei vertexCount = 0;
    };

    std::unordered_map<uint64_t, Mob> m_mobs;
    Mesh m_meshes[4];
    float m_time = 0.0f;
    float m_playerAttackCooldown = 0.0f;

    static uint32_t hash(int x, int z, uint32_t salt = 0);
    static bool isHostile(MobKind kind);
    static glm::vec3 colorFor(MobKind kind);
    void buildMesh(MobKind kind, const TextureAtlas& atlas);
};
