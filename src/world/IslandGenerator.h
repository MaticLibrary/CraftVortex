#pragma once
#include "../engine/Common.h"
#include <FastNoiseLite.h>

class Chunk;
class World;

class IslandGenerator {
public:
    IslandGenerator(int seed = 1337);

    void generateChunkTerrain(Chunk& chunk);
    void decorateChunk(Chunk& chunk, World& world);

    BiomeType getBiomeAt(int worldX, int worldZ) const;
    int getHeightAt(int worldX, int worldZ) const;

    float getIslandRadius() const { return m_islandRadius; }

private:
    int m_seed;
    float m_islandRadius = 3000.0f; // Massive ~6000m island (25-minute walk!)

    FastNoiseLite m_islandMaskNoise;
    FastNoiseLite m_continentalNoise;
    FastNoiseLite m_mountainNoise;
    FastNoiseLite m_detailNoise;
    FastNoiseLite m_biomeNoise;
    FastNoiseLite m_valleyNoise;
    FastNoiseLite m_caveNoise;

    void initNoise();
};
