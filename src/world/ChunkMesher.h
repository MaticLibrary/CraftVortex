#pragma once
#include "../engine/Common.h"
#include "Chunk.h"
#include "../engine/TextureAtlas.h"

class World;

class ChunkMesher {
public:
    static std::vector<VoxelVertex> generateMesh(const Chunk& chunk, const World& world, const TextureAtlas& atlas);

private:
    static BlockId getBlockSafe(const Chunk& chunk, const World& world, int lx, int ly, int lz);
    static int calculateVertexAO(bool side1, bool side2, bool corner);
};
