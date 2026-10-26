#include "IslandGenerator.h"
#include "Chunk.h"
#include "World.h"
#include "Structures.h"

namespace {
uint32_t coordinateHash(int x, int z, uint32_t salt = 0) {
    uint32_t h = static_cast<uint32_t>(x) * 0x8da6b343u;
    h ^= static_cast<uint32_t>(z) * 0xd8163841u;
    h ^= salt + 0xcb1ab31fu;
    h ^= h >> 13;
    h *= 0x85ebca6bu;
    h ^= h >> 16;
    return h;
}

int floorDiv(int value, int divisor) {
    int q = value / divisor;
    if (value % divisor < 0) --q;
    return q;
}

bool isCellCandidate(int chunkX, int chunkZ, int cellSize, uint32_t salt) {
    int cellX = floorDiv(chunkX, cellSize);
    int cellZ = floorDiv(chunkZ, cellSize);
    uint32_t hash = coordinateHash(cellX, cellZ, salt);
    return chunkX == cellX * cellSize + static_cast<int>(hash % cellSize) &&
           chunkZ == cellZ * cellSize + static_cast<int>((hash >> 8) % cellSize);
}

float smoothRange(float value, float start, float end) {
    float t = std::clamp((value - start) / (end - start), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
}

IslandGenerator::IslandGenerator(int seed) : m_seed(seed) {
    initNoise();
}

void IslandGenerator::initNoise() {
    // 1. Coastline organic perturbations
    m_islandMaskNoise.SetSeed(m_seed);
    m_islandMaskNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_islandMaskNoise.SetFrequency(0.0012f);
    m_islandMaskNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_islandMaskNoise.SetFractalOctaves(3);

    // 2. Continental base elevation
    m_continentalNoise.SetSeed(m_seed + 101);
    m_continentalNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_continentalNoise.SetFrequency(0.0025f);
    m_continentalNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_continentalNoise.SetFractalOctaves(4);

    // 3. Mountain ridges
    m_mountainNoise.SetSeed(m_seed + 202);
    m_mountainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_mountainNoise.SetFrequency(0.004f);
    m_mountainNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    m_mountainNoise.SetFractalOctaves(4);

    // 4. Detail hills
    m_detailNoise.SetSeed(m_seed + 303);
    m_detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_detailNoise.SetFrequency(0.015f);
    m_detailNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_detailNoise.SetFractalOctaves(2);

    // 5. Biome selector
    m_biomeNoise.SetSeed(m_seed + 404);
    m_biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    m_biomeNoise.SetFrequency(0.0018f);

    // Broad, winding lowland bands make valleys read as terrain features.
    m_valleyNoise.SetSeed(m_seed + 454);
    m_valleyNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_valleyNoise.SetFrequency(0.0017f);
    m_valleyNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_valleyNoise.SetFractalOctaves(2);

    // 6. 3D Caves
    m_caveNoise.SetSeed(m_seed + 505);
    m_caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_caveNoise.SetFrequency(0.035f);
}

int IslandGenerator::getHeightAt(int worldX, int worldZ) const {
    float dist = std::sqrt((float)(worldX * worldX + worldZ * worldZ));
    float coastPerturb = m_islandMaskNoise.GetNoise((float)worldX, (float)worldZ) * 650.0f;
    float islandEdge = m_islandRadius + coastPerturb;

    if (dist > islandEdge) {
        // Deep ocean outside the island
        float oceanDepth = (dist - islandEdge) * 0.08f;
        int h = SEA_LEVEL - 5 - (int)std::min(oceanDepth, 25.0f);
        return std::max(h, 8);
    }

    // Normalized distance from edge to center (0.0 at coast, 1.0 at center)
    float landFactor = (islandEdge - dist) / islandEdge;
    landFactor = std::clamp(landFactor, 0.0f, 1.0f);

    // Coast transition
    if (landFactor < 0.03f) {
        // Sandy beach transition around sea level
        float beachH = (float)SEA_LEVEL + (landFactor / 0.03f) * 3.0f - 1.0f;
        return (int)beachH;
    }

    // Inland elevation
    float baseH = (float)SEA_LEVEL + 4.0f + landFactor * 22.0f;
    float cont = m_continentalNoise.GetNoise((float)worldX, (float)worldZ) * 13.0f;
    float mount = std::max(0.0f, m_mountainNoise.GetNoise((float)worldX, (float)worldZ));
    float mountHeight = mount * mount * 44.0f * (0.2f + 0.8f * landFactor);
    float detail = m_detailNoise.GetNoise((float)worldX, (float)worldZ) * 3.5f;

    // Carve gentle, connected lowlands through the interior. Attenuating the
    // effect near the coast and peaks keeps shorelines and mountains intact.
    float valley = std::abs(m_valleyNoise.GetNoise((float)worldX, (float)worldZ));
    float valleyFactor = 1.0f - std::clamp(valley / 0.095f, 0.0f, 1.0f);
    float lowlandFactor = 1.0f - std::clamp(mount * 1.8f, 0.0f, 0.85f);
    float valleyDepth = valleyFactor * valleyFactor * 8.0f * lowlandFactor;

    float north = smoothRange(-(float)worldZ, 850.0f, 1400.0f);
    float east = smoothRange((float)worldX, 600.0f, 1450.0f);
    float south = smoothRange((float)worldZ, 150.0f, 850.0f);
    float west = smoothRange(-(float)worldX, 500.0f, 1250.0f);
    float farSouth = 1.0f - smoothRange((float)worldZ, 2200.0f, 2800.0f);
    float frostland = north * (1.0f - east * 0.35f);
    float marshland = east * south * farSouth;
    float emberland = west * south * farSouth;
    float meadowland = east * (1.0f - south * 0.55f);

    // Regional landforms: raised glacial shoulders, low marsh basins, rough
    // volcanic slopes and a calmer rolling meadow shelf.
    float terrainNoise = m_detailNoise.GetNoise((float)worldX, (float)worldZ);
    float regionalShape = frostland * 5.0f - marshland * (4.0f + valleyFactor * 2.0f) +
                          emberland * (mount * 15.0f + std::abs(terrainNoise) * 4.0f) -
                          meadowland * (cont * 0.22f + mountHeight * 0.3f);

    int finalHeight = (int)(baseH + cont + mountHeight + detail - valleyDepth + regionalShape);
    return std::clamp(finalHeight, 15, CHUNK_HEIGHT - 12);
}

BiomeType IslandGenerator::getBiomeAt(int worldX, int worldZ) const {
    int height = getHeightAt(worldX, worldZ);
    if (height < SEA_LEVEL) return BIOME_OCEAN;
    if (height <= SEA_LEVEL + 2) return BIOME_BEACH;
    // Use biome noise and regional quadrants
    float bVal = m_biomeNoise.GetNoise((float)worldX, (float)worldZ);

    if (worldX < -200 && worldZ < -200 && bVal > -0.2f) {
        return BIOME_ANCIENT_DESERT; // Southwest desert & pyramids
    }
    if (abs(worldX) < 600 && abs(worldZ) < 600 && bVal > 0.25f) {
        return BIOME_RUINED_VALLEY; // Central ancient ruined city & valley
    }

    // Climate regions occupy broad, distinct parts of the island, with noise
    // breaking their borders into natural-looking fingers and pockets.
    if (worldZ < -1050 && bVal > -0.75f) return BIOME_FROSTFALL;
    if (worldX > 650 && worldZ > 250 && bVal < 0.45f) return BIOME_MISTY_MARSH;
    if (worldX < -650 && worldZ > 250 && bVal < 0.35f) return BIOME_EMBERLANDS;
    if (worldX > 650 && worldZ < 500 && bVal < 0.35f) return BIOME_MOONLIT_MEADOW;
    if (height > 82) return BIOME_CRYSTAL_PEAKS;

    return BIOME_SPOOKY_FOREST; // Sprawling dark mystical forest
}

void IslandGenerator::generateChunkTerrain(Chunk& chunk) {
    int startWX = chunk.chunkX * CHUNK_SIZE_X;
    int startWZ = chunk.chunkZ * CHUNK_SIZE_Z;

    for (int lx = 0; lx < CHUNK_SIZE_X; ++lx) {
        for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz) {
            int wx = startWX + lx;
            int wz = startWZ + lz;

            int surfaceH = getHeightAt(wx, wz);
            BiomeType biome = getBiomeAt(wx, wz);
            float localClimate = m_biomeNoise.GetNoise((float)wx, (float)wz);

            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                // Bedrock base
                if (y == 0) {
                    chunk.setBlock(lx, y, lz, BLOCK_OBSIDIAN);
                    continue;
                }

                // Air above surface and above sea level
                if (y > surfaceH) {
                    bool marshPool = biome == BIOME_MISTY_MARSH && localClimate > 0.28f && y <= surfaceH + 1;
                    if (y <= SEA_LEVEL || marshPool) {
                        chunk.setBlock(lx, y, lz, BLOCK_WATER);
                    } else {
                        chunk.setBlock(lx, y, lz, BLOCK_AIR);
                    }
                    continue;
                }

                // 3D Cave carving (don't carve too close to surface or near sea level to avoid flooding)
                if (y > 8 && y < surfaceH - 4 && y != SEA_LEVEL && y != SEA_LEVEL + 1) {
                    float cave = m_caveNoise.GetNoise((float)wx, (float)y * 1.5f, (float)wz);
                    if (cave > 0.52f) {
                        chunk.setBlock(lx, y, lz, BLOCK_AIR);
                        continue;
                    }
                }

                // Terrain layers based on depth
                int depth = surfaceH - y;

                if (depth == 0) {
                    // Surface block
                    if (surfaceH < SEA_LEVEL + 2) {
                        chunk.setBlock(lx, y, lz, BLOCK_SAND);
                    } else if (biome == BIOME_ANCIENT_DESERT) {
                        chunk.setBlock(lx, y, lz, (lx + lz) % 3 == 0 ? BLOCK_SAND : BLOCK_SANDSTONE);
                    } else if (biome == BIOME_FROSTFALL) {
                        chunk.setBlock(lx, y, lz, BLOCK_SNOW);
                    } else if (biome == BIOME_MISTY_MARSH) {
                        chunk.setBlock(lx, y, lz, localClimate > 0.1f ? BLOCK_MARSH_MUD : BLOCK_MOSS_GRASS);
                    } else if (biome == BIOME_EMBERLANDS) {
                        chunk.setBlock(lx, y, lz, (localClimate > 0.15f) ? BLOCK_BASALT : BLOCK_OBSIDIAN);
                    } else if (biome == BIOME_MOONLIT_MEADOW) {
                        chunk.setBlock(lx, y, lz, BLOCK_MEADOW_GRASS);
                    } else if (biome == BIOME_CRYSTAL_PEAKS) {
                        chunk.setBlock(lx, y, lz, BLOCK_STONE);
                    } else {
                        chunk.setBlock(lx, y, lz, BLOCK_GRASS);
                    }
                } else if (depth <= 3) {
                    // Subsurface
                    if (biome == BIOME_ANCIENT_DESERT) {
                        chunk.setBlock(lx, y, lz, BLOCK_SANDSTONE);
                    } else if (biome == BIOME_FROSTFALL) {
                        chunk.setBlock(lx, y, lz, depth == 1 ? BLOCK_SNOW : BLOCK_STONE);
                    } else if (biome == BIOME_MISTY_MARSH) {
                        chunk.setBlock(lx, y, lz, BLOCK_MARSH_MUD);
                    } else if (biome == BIOME_EMBERLANDS) {
                        chunk.setBlock(lx, y, lz, BLOCK_BASALT);
                    } else if (biome == BIOME_MOONLIT_MEADOW) {
                        chunk.setBlock(lx, y, lz, BLOCK_DIRT);
                    } else if (surfaceH < SEA_LEVEL + 2) {
                        chunk.setBlock(lx, y, lz, BLOCK_SAND);
                    } else {
                        chunk.setBlock(lx, y, lz, BLOCK_DIRT);
                    }
                } else {
                    // Deep underground stone with fantasy ores
                    int oreHash = abs(wx * 37 + y * 73 + wz * 91);
                    if (oreHash % 97 == 0 && y < 45) {
                        chunk.setBlock(lx, y, lz, BLOCK_RUNE_STONE); // Rare ancient glowing rune
                    } else if (oreHash % 41 == 0 && y < 35) {
                        chunk.setBlock(lx, y, lz, BLOCK_AMETHYST); // Underground amethyst crystal
                    } else if (oreHash % 53 == 0 && y < 30) {
                        chunk.setBlock(lx, y, lz, BLOCK_EMERALD_MANA); // Deep mana vein
                    } else if (oreHash % 29 == 0 && y < 50) {
                        chunk.setBlock(lx, y, lz, BLOCK_GOLD_ORE); // Gold
                    } else {
                        chunk.setBlock(lx, y, lz, BLOCK_STONE);
                    }
                }
            }
        }
    }

    chunk.isGenerated = true;
    chunk.isDirty = true;
}

void IslandGenerator::decorateChunk(Chunk& chunk, World& world) {
    int startWX = chunk.chunkX * CHUNK_SIZE_X;
    int startWZ = chunk.chunkZ * CHUNK_SIZE_Z;

    int centerWX = startWX + CHUNK_SIZE_X / 2;
    int centerWZ = startWZ + CHUNK_SIZE_Z / 2;
    BiomeType biome = getBiomeAt(centerWX, centerWZ);

    auto setWorldSafe = [&](int wx, int wy, int wz, BlockId id) {
        if (wy >= 0 && wy < CHUNK_HEIGHT) {
            world.setBlock(wx, wy, wz, id);
            if (id == BLOCK_CHEST) world.populateChestLoot(glm::ivec3(wx, wy, wz));
        }
    };

    auto suitableSite = [&](int x, int z, int radius, int maxSlope) {
        int base = getHeightAt(x, z);
        if (base <= SEA_LEVEL + 2 || base >= CHUNK_HEIGHT - 20) return false;
        for (int dx = -radius; dx <= radius; dx += std::max(1, radius / 3)) {
            for (int dz = -radius; dz <= radius; dz += std::max(1, radius / 3)) {
                if (std::abs(getHeightAt(x + dx, z + dz) - base) > maxSlope) return false;
                BiomeType nearby = getBiomeAt(x + dx, z + dz);
                if (nearby == BIOME_OCEAN || nearby == BIOME_BEACH) return false;
            }
        }
        return true;
    };

    uint32_t hash = coordinateHash(chunk.chunkX, chunk.chunkZ);

    // Dense, irregular forest growth with a few trees per chunk.
    if (biome == BIOME_SPOOKY_FOREST) {
        int treeCount = 1 + static_cast<int>((hash >> 5) % 3);
        for (int i = 0; i < treeCount; ++i) {
            uint32_t treeHash = coordinateHash(chunk.chunkX, chunk.chunkZ, 0x91e10da5u + i);
            if (treeHash % 5 == 0) continue;
            int tx = startWX + 2 + static_cast<int>((treeHash >> 8) % 12);
            int tz = startWZ + 2 + static_cast<int>((treeHash >> 16) % 12);
            int ty = getHeightAt(tx, tz);
            if (ty > SEA_LEVEL + 1 && ty < CHUNK_HEIGHT - 25 && getBiomeAt(tx, tz) == BIOME_SPOOKY_FOREST)
                Structures::buildSpookyTree(tx, ty + 1, tz, setWorldSafe);
        }
    }

    if (biome == BIOME_FROSTFALL && hash % 3u != 0) {
        int tx = startWX + 3 + static_cast<int>((hash >> 8) % 10);
        int tz = startWZ + 3 + static_cast<int>((hash >> 16) % 10);
        int ty = getHeightAt(tx, tz);
        if (ty > SEA_LEVEL + 1 && ty < CHUNK_HEIGHT - 18)
            Structures::buildFrozenPine(tx, ty + 1, tz, setWorldSafe);
    }

    if (biome == BIOME_MISTY_MARSH && hash % 2u == 0) {
        int tx = startWX + 3 + static_cast<int>((hash >> 8) % 10);
        int tz = startWZ + 3 + static_cast<int>((hash >> 16) % 10);
        int ty = getHeightAt(tx, tz);
        if (ty > SEA_LEVEL && ty < CHUNK_HEIGHT - 16)
            Structures::buildBogTree(tx, ty + 1, tz, setWorldSafe);
    }

    if (biome == BIOME_EMBERLANDS && hash % 13u == 0 &&
        suitableSite(centerWX, centerWZ, 6, 5)) {
        Structures::buildVolcanicVent(centerWX, getHeightAt(centerWX, centerWZ) + 1, centerWZ, setWorldSafe);
    }

    if (biome == BIOME_MOONLIT_MEADOW && hash % 9u == 0 &&
        suitableSite(centerWX, centerWZ, 5, 3)) {
        Structures::buildMeadowRuin(centerWX, getHeightAt(centerWX, centerWZ) + 1, centerWZ, setWorldSafe);
    }

    // Rare settlements use fixed macro-cells, so chunk load order cannot alter them.
    uint32_t siteHash = coordinateHash(chunk.chunkX, chunk.chunkZ, 0x4f1bbcdcu);
    // Guaranteed landmarks close to the starting region make the structures
    // easy to discover in a fresh world; the rest remain procedurally scattered.
    bool starterVillage = chunk.chunkX == -5 && chunk.chunkZ == 4;
    if (starterVillage) {
        Structures::buildVillage(centerWX, getHeightAt(centerWX, centerWZ) + 1, centerWZ, setWorldSafe);
    } else if ((biome == BIOME_SPOOKY_FOREST || biome == BIOME_RUINED_VALLEY) &&
        isCellCandidate(chunk.chunkX, chunk.chunkZ, 20, 0x7654321u) && (siteHash % 3 != 0)) {
        int y = getHeightAt(centerWX, centerWZ);
        if (suitableSite(centerWX, centerWZ, 14, 3))
            Structures::buildVillage(centerWX, y + 1, centerWZ, setWorldSafe);
    }

    // Desert shrines and stepped pyramids appear at separated landmarks.
    if (biome == BIOME_ANCIENT_DESERT) {
        if (isCellCandidate(chunk.chunkX, chunk.chunkZ, 30, 0x21f0aaadu) &&
            suitableSite(centerWX, centerWZ, 9, 3)) {
            Structures::buildTemple(centerWX, getHeightAt(centerWX, centerWZ) + 1, centerWZ, setWorldSafe);
        } else if (chunk.chunkX % 14 == 0 && chunk.chunkZ % 14 == 0 &&
                   suitableSite(centerWX, centerWZ, 10, 4)) {
            Structures::buildPyramid(centerWX, getHeightAt(centerWX, centerWZ) + 1, centerWZ, setWorldSafe);
        }
    }

    // A handful of large castles mark the ancient valley region.
    bool starterCastle = chunk.chunkX == 3 && chunk.chunkZ == 2;
    if (starterCastle) {
        int castleFloor = getHeightAt(centerWX, centerWZ) + 1;
        // Stone piers follow the irregular ground under the broad fortress
        // platform, anchoring the silhouette into the landscape.
        for (int dx = -24; dx <= 24; dx += 6) {
            for (int dz = -24; dz <= 24; dz += 6) {
                int ground = getHeightAt(centerWX + dx, centerWZ + dz);
                for (int y = ground + 1; y < castleFloor; ++y)
                    setWorldSafe(centerWX + dx, y, centerWZ + dz, BLOCK_COBBLESTONE);
            }
        }
        Structures::buildGothicCastle(centerWX, castleFloor, centerWZ, setWorldSafe);
    } else if (biome == BIOME_RUINED_VALLEY &&
        isCellCandidate(chunk.chunkX, chunk.chunkZ, 42, 0xca571e3u) &&
        suitableSite(centerWX, centerWZ, 24, 10)) {
        Structures::buildGothicCastle(centerWX, getHeightAt(centerWX, centerWZ) + 1, centerWZ, setWorldSafe);
    }

    // Ruined watchtowers remain scattered through the old valley.
    if (biome == BIOME_RUINED_VALLEY && (hash % 17u == 0)) {
        int rx = startWX + 8;
        int rz = startWZ + 8;
        if (suitableSite(rx, rz, 4, 5))
            Structures::buildRuinedTower(rx, getHeightAt(rx, rz) + 1, rz, setWorldSafe);
    }

    // Crystal spires are concentrated on high ground.
    if (biome == BIOME_CRYSTAL_PEAKS && hash % 4u == 0) {
        int cx = startWX + 3 + static_cast<int>((hash >> 8) % 10);
        int cz = startWZ + 3 + static_cast<int>((hash >> 16) % 10);
        int cy = getHeightAt(cx, cz);
        if (cy > SEA_LEVEL && cy < CHUNK_HEIGHT - 15)
            Structures::buildCrystalSpire(cx, cy + 1, cz, setWorldSafe);
    }
}
