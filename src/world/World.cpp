#include "World.h"
#include <iostream>

World::World() : m_generator(1337) {}

World::~World() {}

uint64_t World::packBlockKey(const glm::ivec3& position) {
    constexpr uint64_t coordinateMask = (1u << 21) - 1u;
    uint64_t x = static_cast<uint64_t>(position.x + (1 << 20)) & coordinateMask;
    uint64_t z = static_cast<uint64_t>(position.z + (1 << 20)) & coordinateMask;
    uint64_t y = static_cast<uint64_t>(position.y) & 0x7fu;
    return (x << 28) | (z << 7) | y;
}

ChestInventory* World::getChestInventory(const glm::ivec3& blockPosition) {
    if (getBlock(blockPosition.x, blockPosition.y, blockPosition.z) != BLOCK_CHEST) return nullptr;
    return &m_chestInventories.try_emplace(packBlockKey(blockPosition)).first->second;
}

const ChestInventory* World::getChestInventory(const glm::ivec3& blockPosition) const {
    return const_cast<World*>(this)->getChestInventory(blockPosition);
}

void World::populateChestLoot(const glm::ivec3& blockPosition) {
    ChestInventory* chest = getChestInventory(blockPosition);
    if (!chest || chest->generatedLootPopulated) return;

    uint32_t seed = static_cast<uint32_t>(blockPosition.x) * 0x8da6b343u ^
                    static_cast<uint32_t>(blockPosition.z) * 0xd8163841u ^
                    static_cast<uint32_t>(blockPosition.y) * 0xcb1ab31fu;
    seed ^= seed >> 13;
    seed *= 0x85ebca6bu;
    chest->blocks[BLOCK_GOLD_ORE] = 1 + seed % 4;
    chest->blocks[BLOCK_AMETHYST] = 1 + ((seed >> 5) % 3);
    chest->arrows = 6 + ((seed >> 11) % 15);
    if (seed % 5 == 0) chest->blocks[BLOCK_RUNE_STONE] = 1;
    if (seed % 7 == 0) chest->blocks[BLOCK_CRYSTAL_BRICK] = 2;
    chest->generatedLootPopulated = true;
}

void World::init() {
    m_textureAtlas.generate();
    std::cout << "[World] Texture Atlas generated successfully.\n";
}

int World::getSpawnHeight(int wx, int wz) const {
    return m_generator.getHeightAt(wx, wz);
}

BiomeType World::getBiome(int wx, int wz) const {
    return m_generator.getBiomeAt(wx, wz);
}

BiomeAtmosphere World::getAtmosphere(const glm::vec3& playerPos) const {
    BiomeType b = getBiome((int)playerPos.x, (int)playerPos.z);
    switch (b) {
        case BIOME_SPOOKY_FOREST:
            return { glm::vec3(0.18f, 0.12f, 0.28f), 20.0f, 150.0f, "Nawiedzony Las (Spooky Forest)" };
        case BIOME_ANCIENT_DESERT:
            return { glm::vec3(0.82f, 0.70f, 0.45f), 30.0f, 180.0f, "Pustynia Pradawnych & Piramidy" };
        case BIOME_CRYSTAL_PEAKS:
            return { glm::vec3(0.32f, 0.40f, 0.65f), 35.0f, 200.0f, "Szczyty Magicznych Kryształów" };
        case BIOME_RUINED_VALLEY:
            return { glm::vec3(0.35f, 0.40f, 0.38f), 25.0f, 160.0f, "Dolina Ruin & Pradawne Miasto" };
        case BIOME_FROSTFALL:
            return { glm::vec3(0.66f, 0.78f, 0.91f), 28.0f, 190.0f, "Mroźna Knieja" };
        case BIOME_MISTY_MARSH:
            return { glm::vec3(0.34f, 0.52f, 0.43f), 18.0f, 125.0f, "Bagna Szeptów" };
        case BIOME_EMBERLANDS:
            return { glm::vec3(0.52f, 0.27f, 0.20f), 24.0f, 155.0f, "Pola Żaru" };
        case BIOME_MOONLIT_MEADOW:
            return { glm::vec3(0.46f, 0.60f, 0.70f), 32.0f, 205.0f, "Srebrzyste Łąki" };
        case BIOME_BEACH:
        case BIOME_OCEAN:
        default:
            return { glm::vec3(0.45f, 0.68f, 0.88f), 40.0f, 220.0f, "Mistyczny Ocean & Wybrzeże" };
    }
}

Chunk* World::getChunk(int cx, int cz) {
    uint64_t key = packChunkKey(cx, cz);
    auto it = m_chunks.find(key);
    if (it != m_chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Chunk* World::getChunk(int cx, int cz) const {
    uint64_t key = packChunkKey(cx, cz);
    auto it = m_chunks.find(key);
    if (it != m_chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

BlockId World::getBlock(int wx, int wy, int wz) const {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return BLOCK_AIR;

    int cx = (int)std::floor((float)wx / (float)CHUNK_SIZE_X);
    int cz = (int)std::floor((float)wz / (float)CHUNK_SIZE_Z);

    int lx = wx - cx * CHUNK_SIZE_X;
    int lz = wz - cz * CHUNK_SIZE_Z;

    const Chunk* chunk = getChunk(cx, cz);
    if (chunk) {
        return chunk->getBlock(lx, wy, lz);
    }

    return BLOCK_AIR;
}

void World::setBlock(int wx, int wy, int wz, BlockId id) {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return;

    int cx = (int)std::floor((float)wx / (float)CHUNK_SIZE_X);
    int cz = (int)std::floor((float)wz / (float)CHUNK_SIZE_Z);

    int lx = wx - cx * CHUNK_SIZE_X;
    int lz = wz - cz * CHUNK_SIZE_Z;

    Chunk* chunk = getChunk(cx, cz);
    if (!chunk) {
        loadChunk(cx, cz);
        chunk = getChunk(cx, cz);
    }

    if (chunk) {
        BlockId oldBlock = chunk->getBlock(lx, wy, lz);
        if (oldBlock == BLOCK_CHEST && id != BLOCK_CHEST)
            m_chestInventories.erase(packBlockKey(glm::ivec3(wx, wy, wz)));
        chunk->setBlock(lx, wy, lz, id);

        // Mark neighboring chunks dirty if block is on chunk boundary
        if (lx == 0) {
            Chunk* nb = getChunk(cx - 1, cz);
            if (nb) nb->isDirty = true;
        } else if (lx == CHUNK_SIZE_X - 1) {
            Chunk* nb = getChunk(cx + 1, cz);
            if (nb) nb->isDirty = true;
        }

        if (lz == 0) {
            Chunk* nb = getChunk(cx, cz - 1);
            if (nb) nb->isDirty = true;
        } else if (lz == CHUNK_SIZE_Z - 1) {
            Chunk* nb = getChunk(cx, cz + 1);
            if (nb) nb->isDirty = true;
        }
    }
}

void World::loadChunk(int cx, int cz) {
    uint64_t key = packChunkKey(cx, cz);
    if (m_chunks.find(key) != m_chunks.end()) return;

    auto newChunk = std::make_unique<Chunk>(cx, cz);
    m_generator.generateChunkTerrain(*newChunk);
    m_chunks[key] = std::move(newChunk);
}

void World::update(const glm::vec3& playerPos) {
    int pChunkX = (int)std::floor(playerPos.x / (float)CHUNK_SIZE_X);
    int pChunkZ = (int)std::floor(playerPos.z / (float)CHUNK_SIZE_Z);

    // 1. Load chunks in render distance
    int r = WORLD_RENDER_DISTANCE;
    for (int dz = -r; dz <= r; ++dz) {
        for (int dx = -r; dx <= r; ++dx) {
            if (dx * dx + dz * dz <= (r + 1) * (r + 1)) {
                int cx = pChunkX + dx;
                int cz = pChunkZ + dz;
                loadChunk(cx, cz);
            }
        }
    }

    // 2. Decorate chunks that need decoration
    for (int dz = -r + 1; dz <= r - 1; ++dz) {
        for (int dx = -r + 1; dx <= r - 1; ++dx) {
            int cx = pChunkX + dx;
            int cz = pChunkZ + dz;
            Chunk* c = getChunk(cx, cz);
            if (c && c->isGenerated) {
                // Decorate
                m_generator.decorateChunk(*c, *this);
                c->isGenerated = false; // Flag as decorated
            }
        }
    }

    // 3. Mesh rebuilding (budget per frame to guarantee 60-144 FPS)
    int meshedCount = 0;
    int meshBudget = 6;

    for (int dist = 0; dist <= r; ++dist) {
        for (int dz = -dist; dz <= dist; ++dz) {
            for (int dx = -dist; dx <= dist; ++dx) {
                if (std::max(abs(dx), abs(dz)) != dist) continue;
                int cx = pChunkX + dx;
                int cz = pChunkZ + dz;
                Chunk* c = getChunk(cx, cz);
                if (c && c->isDirty) {
                    c->buildMesh(*this);
                    meshedCount++;
                    if (meshedCount >= meshBudget) break;
                }
            }
            if (meshedCount >= meshBudget) break;
        }
        if (meshedCount >= meshBudget) break;
    }

    // 4. Memory management: unload distant chunks beyond render distance + 3
    if (pChunkX != m_lastPlayerChunkX || pChunkZ != m_lastPlayerChunkZ) {
        m_lastPlayerChunkX = pChunkX;
        m_lastPlayerChunkZ = pChunkZ;

        int unloadDist = r + 3;
        for (auto it = m_chunks.begin(); it != m_chunks.end();) {
            int cx = it->second->chunkX;
            int cz = it->second->chunkZ;
            int dx = cx - pChunkX;
            int dz = cz - pChunkZ;
            if (dx * dx + dz * dz > unloadDist * unloadDist) {
                it = m_chunks.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void World::render(const Shader& shader, const glm::vec3& playerPos) {
    m_textureAtlas.bind(0);

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("uModel", model);

    int pChunkX = (int)std::floor(playerPos.x / (float)CHUNK_SIZE_X);
    int pChunkZ = (int)std::floor(playerPos.z / (float)CHUNK_SIZE_Z);
    int r = WORLD_RENDER_DISTANCE;

    for (int dz = -r; dz <= r; ++dz) {
        for (int dx = -r; dx <= r; ++dx) {
            if (dx * dx + dz * dz <= (r + 1) * (r + 1)) {
                Chunk* c = getChunk(pChunkX + dx, pChunkZ + dz);
                if (c) {
                    c->render();
                }
            }
        }
    }
}
