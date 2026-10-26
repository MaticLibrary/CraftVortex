#pragma once
#include "../engine/Common.h"
#include "Chunk.h"
#include "IslandGenerator.h"
#include "../engine/TextureAtlas.h"
#include "../engine/Shader.h"
#include <array>

struct ChestInventory {
    std::array<int, BLOCK_COUNT> blocks{};
    int arrows = 0;
    bool generatedLootPopulated = false;
};

struct BiomeAtmosphere {
    glm::vec3 fogColor;
    float fogStart;
    float fogEnd;
    std::string displayName;
};

class World {
public:
    World();
    ~World();

    void init();
    void update(const glm::vec3& playerPos);
    void render(const Shader& shader, const glm::vec3& playerPos);

    BlockId getBlock(int wx, int wy, int wz) const;
    void setBlock(int wx, int wy, int wz, BlockId id);

    Chunk* getChunk(int cx, int cz);
    const Chunk* getChunk(int cx, int cz) const;

    const TextureAtlas& getTextureAtlas() const { return m_textureAtlas; }
    BiomeAtmosphere getAtmosphere(const glm::vec3& playerPos) const;
    BiomeType getBiome(int wx, int wz) const;
    int getSpawnHeight(int wx, int wz) const;
    ChestInventory* getChestInventory(const glm::ivec3& blockPosition);
    const ChestInventory* getChestInventory(const glm::ivec3& blockPosition) const;
    void populateChestLoot(const glm::ivec3& blockPosition);

    static uint64_t packChunkKey(int cx, int cz) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32) |
               static_cast<uint64_t>(static_cast<uint32_t>(cz));
    }

private:
    std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_chunks;
    mutable std::unordered_map<uint64_t, ChestInventory> m_chestInventories;
    TextureAtlas m_textureAtlas;
    IslandGenerator m_generator;

    int m_lastPlayerChunkX = 999999;
    int m_lastPlayerChunkZ = 999999;

    void loadChunk(int cx, int cz);
    static uint64_t packBlockKey(const glm::ivec3& position);
};
