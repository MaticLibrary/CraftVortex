#include "ChunkMesher.h"
#include "World.h"

int ChunkMesher::calculateVertexAO(bool side1, bool side2, bool corner) {
    if (side1 && side2) {
        return 0;
    }
    return 3 - (side1 + side2 + corner);
}

BlockId ChunkMesher::getBlockSafe(const Chunk& chunk, const World& world, int lx, int ly, int lz) {
    if (ly < 0 || ly >= CHUNK_HEIGHT) return BLOCK_AIR;
    if (Chunk::inBounds(lx, ly, lz)) {
        return chunk.getBlock(lx, ly, lz);
    }
    int wx = chunk.chunkX * CHUNK_SIZE_X + lx;
    int wz = chunk.chunkZ * CHUNK_SIZE_Z + lz;
    return world.getBlock(wx, ly, wz);
}

std::vector<VoxelVertex> ChunkMesher::generateMesh(const Chunk& chunk, const World& world, const TextureAtlas& atlas) {
    std::vector<VoxelVertex> vertices;
    vertices.reserve(12000);

    auto isSolidBlock = [&](int lx, int ly, int lz) -> bool {
        BlockId b = getBlockSafe(chunk, world, lx, ly, lz);
        return BlockRegistry::get(b).isSolid;
    };

    auto isTransparentBlock = [&](int lx, int ly, int lz) -> bool {
        BlockId b = getBlockSafe(chunk, world, lx, ly, lz);
        return BlockRegistry::get(b).isTransparent;
    };

    float wx0 = (float)(chunk.chunkX * CHUNK_SIZE_X);
    float wz0 = (float)(chunk.chunkZ * CHUNK_SIZE_Z);

    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                BlockId id = chunk.getBlock(x, y, z);
                if (id == BLOCK_AIR) continue;

                const BlockData& data = BlockRegistry::get(id);
                float emissive = data.isEmissive ? 1.0f : 0.0f;
                glm::vec3 emitCol(data.emitR, data.emitG, data.emitB);

                float fx = wx0 + x;
                float fy = (float)y;
                float fz = wz0 + z;

                float u0, v0, u1, v1;

                // Chains are thin, non-colliding prisms rather than opaque
                // full cubes, so they read as hanging links in rooms.
                if (id == BLOCK_IRON_CHAIN) {
                    auto chainFace = [&](const glm::vec3& a, const glm::vec3& b,
                                         const glm::vec3& c, const glm::vec3& d,
                                         const glm::vec3& normal, int tile) {
                        atlas.getTileUVs(tile, u0, v0, u1, v1);
                        VoxelVertex va{a, {u0, v1}, normal, 3.0f, emissive, emitCol};
                        VoxelVertex vb{b, {u1, v1}, normal, 3.0f, emissive, emitCol};
                        VoxelVertex vc{c, {u1, v0}, normal, 3.0f, emissive, emitCol};
                        VoxelVertex vd{d, {u0, v0}, normal, 3.0f, emissive, emitCol};
                        vertices.insert(vertices.end(), {va, vb, vc, va, vc, vd});
                    };
                    float x0 = fx + 0.41f, x1 = fx + 0.59f;
                    float z0 = fz + 0.41f, z1 = fz + 0.59f;
                    chainFace({x1, fy, z0}, {x1, fy + 1, z0}, {x1, fy + 1, z1}, {x1, fy, z1}, { 1, 0, 0}, data.textures.right);
                    chainFace({x0, fy, z1}, {x0, fy + 1, z1}, {x0, fy + 1, z0}, {x0, fy, z0}, {-1, 0, 0}, data.textures.left);
                    chainFace({x0, fy, z1}, {x1, fy, z1}, {x1, fy + 1, z1}, {x0, fy + 1, z1}, { 0, 0, 1}, data.textures.front);
                    chainFace({x0, fy, z0}, {x0, fy + 1, z0}, {x1, fy + 1, z0}, {x1, fy, z0}, { 0, 0,-1}, data.textures.back);
                    if (getBlockSafe(chunk, world, x, y + 1, z) != BLOCK_IRON_CHAIN)
                        chainFace({x0, fy + 1, z0}, {x0, fy + 1, z1}, {x1, fy + 1, z1}, {x1, fy + 1, z0}, {0, 1, 0}, data.textures.top);
                    if (getBlockSafe(chunk, world, x, y - 1, z) != BLOCK_IRON_CHAIN)
                        chainFace({x0, fy, z1}, {x0, fy, z0}, {x1, fy, z0}, {x1, fy, z1}, {0, -1, 0}, data.textures.bottom);
                    continue;
                }

                // --- 1. TOP FACE (+Y) ---
                if (isTransparentBlock(x, y + 1, z)) {
                    atlas.getTileUVs(data.textures.top, u0, v0, u1, v1);
                    glm::vec3 norm(0.0f, 1.0f, 0.0f);

                    float ao0 = (float)calculateVertexAO(isSolidBlock(x - 1, y + 1, z), isSolidBlock(x, y + 1, z - 1), isSolidBlock(x - 1, y + 1, z - 1));
                    float ao1 = (float)calculateVertexAO(isSolidBlock(x - 1, y + 1, z), isSolidBlock(x, y + 1, z + 1), isSolidBlock(x - 1, y + 1, z + 1));
                    float ao2 = (float)calculateVertexAO(isSolidBlock(x + 1, y + 1, z), isSolidBlock(x, y + 1, z + 1), isSolidBlock(x + 1, y + 1, z + 1));
                    float ao3 = (float)calculateVertexAO(isSolidBlock(x + 1, y + 1, z), isSolidBlock(x, y + 1, z - 1), isSolidBlock(x + 1, y + 1, z - 1));

                    VoxelVertex vt0{ {fx,     fy + 1, fz    }, {u0, v0}, norm, ao0, emissive, emitCol };
                    VoxelVertex vt1{ {fx,     fy + 1, fz + 1}, {u0, v1}, norm, ao1, emissive, emitCol };
                    VoxelVertex vt2{ {fx + 1, fy + 1, fz + 1}, {u1, v1}, norm, ao2, emissive, emitCol };
                    VoxelVertex vt3{ {fx + 1, fy + 1, fz    }, {u1, v0}, norm, ao3, emissive, emitCol };

                    if (ao0 + ao2 > ao1 + ao3) {
                        vertices.push_back(vt0); vertices.push_back(vt1); vertices.push_back(vt2);
                        vertices.push_back(vt0); vertices.push_back(vt2); vertices.push_back(vt3);
                    } else {
                        vertices.push_back(vt1); vertices.push_back(vt2); vertices.push_back(vt3);
                        vertices.push_back(vt1); vertices.push_back(vt3); vertices.push_back(vt0);
                    }
                }

                // --- 2. BOTTOM FACE (-Y) ---
                if (isTransparentBlock(x, y - 1, z)) {
                    atlas.getTileUVs(data.textures.bottom, u0, v0, u1, v1);
                    glm::vec3 norm(0.0f, -1.0f, 0.0f);

                    float ao0 = (float)calculateVertexAO(isSolidBlock(x - 1, y - 1, z), isSolidBlock(x, y - 1, z - 1), isSolidBlock(x - 1, y - 1, z - 1));
                    float ao1 = (float)calculateVertexAO(isSolidBlock(x + 1, y - 1, z), isSolidBlock(x, y - 1, z - 1), isSolidBlock(x + 1, y - 1, z - 1));
                    float ao2 = (float)calculateVertexAO(isSolidBlock(x + 1, y - 1, z), isSolidBlock(x, y - 1, z + 1), isSolidBlock(x + 1, y - 1, z + 1));
                    float ao3 = (float)calculateVertexAO(isSolidBlock(x - 1, y - 1, z), isSolidBlock(x, y - 1, z + 1), isSolidBlock(x - 1, y - 1, z + 1));

                    VoxelVertex vt0{ {fx,     fy, fz    }, {u0, v0}, norm, ao0, emissive, emitCol };
                    VoxelVertex vt1{ {fx + 1, fy, fz    }, {u1, v0}, norm, ao1, emissive, emitCol };
                    VoxelVertex vt2{ {fx + 1, fy, fz + 1}, {u1, v1}, norm, ao2, emissive, emitCol };
                    VoxelVertex vt3{ {fx,     fy, fz + 1}, {u0, v1}, norm, ao3, emissive, emitCol };

                    if (ao0 + ao2 > ao1 + ao3) {
                        vertices.push_back(vt0); vertices.push_back(vt1); vertices.push_back(vt2);
                        vertices.push_back(vt0); vertices.push_back(vt2); vertices.push_back(vt3);
                    } else {
                        vertices.push_back(vt1); vertices.push_back(vt2); vertices.push_back(vt3);
                        vertices.push_back(vt1); vertices.push_back(vt3); vertices.push_back(vt0);
                    }
                }

                // --- 3. NORTH FACE (+Z) ---
                if (isTransparentBlock(x, y, z + 1)) {
                    atlas.getTileUVs(data.textures.front, u0, v0, u1, v1);
                    glm::vec3 norm(0.0f, 0.0f, 1.0f);

                    float ao0 = (float)calculateVertexAO(isSolidBlock(x - 1, y, z + 1), isSolidBlock(x, y - 1, z + 1), isSolidBlock(x - 1, y - 1, z + 1));
                    float ao1 = (float)calculateVertexAO(isSolidBlock(x + 1, y, z + 1), isSolidBlock(x, y - 1, z + 1), isSolidBlock(x + 1, y - 1, z + 1));
                    float ao2 = (float)calculateVertexAO(isSolidBlock(x + 1, y, z + 1), isSolidBlock(x, y + 1, z + 1), isSolidBlock(x + 1, y + 1, z + 1));
                    float ao3 = (float)calculateVertexAO(isSolidBlock(x - 1, y, z + 1), isSolidBlock(x, y + 1, z + 1), isSolidBlock(x - 1, y + 1, z + 1));

                    VoxelVertex vt0{ {fx,     fy,     fz + 1}, {u0, v1}, norm, ao0, emissive, emitCol };
                    VoxelVertex vt1{ {fx + 1, fy,     fz + 1}, {u1, v1}, norm, ao1, emissive, emitCol };
                    VoxelVertex vt2{ {fx + 1, fy + 1, fz + 1}, {u1, v0}, norm, ao2, emissive, emitCol };
                    VoxelVertex vt3{ {fx,     fy + 1, fz + 1}, {u0, v0}, norm, ao3, emissive, emitCol };

                    if (ao0 + ao2 > ao1 + ao3) {
                        vertices.push_back(vt0); vertices.push_back(vt1); vertices.push_back(vt2);
                        vertices.push_back(vt0); vertices.push_back(vt2); vertices.push_back(vt3);
                    } else {
                        vertices.push_back(vt1); vertices.push_back(vt2); vertices.push_back(vt3);
                        vertices.push_back(vt1); vertices.push_back(vt3); vertices.push_back(vt0);
                    }
                }

                // --- 4. SOUTH FACE (-Z) ---
                if (isTransparentBlock(x, y, z - 1)) {
                    atlas.getTileUVs(data.textures.back, u0, v0, u1, v1);
                    glm::vec3 norm(0.0f, 0.0f, -1.0f);

                    float ao0 = (float)calculateVertexAO(isSolidBlock(x + 1, y, z - 1), isSolidBlock(x, y - 1, z - 1), isSolidBlock(x + 1, y - 1, z - 1));
                    float ao1 = (float)calculateVertexAO(isSolidBlock(x - 1, y, z - 1), isSolidBlock(x, y - 1, z - 1), isSolidBlock(x - 1, y - 1, z - 1));
                    float ao2 = (float)calculateVertexAO(isSolidBlock(x - 1, y, z - 1), isSolidBlock(x, y + 1, z - 1), isSolidBlock(x - 1, y + 1, z - 1));
                    float ao3 = (float)calculateVertexAO(isSolidBlock(x + 1, y, z - 1), isSolidBlock(x, y + 1, z - 1), isSolidBlock(x + 1, y + 1, z - 1));

                    VoxelVertex vt0{ {fx + 1, fy,     fz}, {u0, v1}, norm, ao0, emissive, emitCol };
                    VoxelVertex vt1{ {fx,     fy,     fz}, {u1, v1}, norm, ao1, emissive, emitCol };
                    VoxelVertex vt2{ {fx,     fy + 1, fz}, {u1, v0}, norm, ao2, emissive, emitCol };
                    VoxelVertex vt3{ {fx + 1, fy + 1, fz}, {u0, v0}, norm, ao3, emissive, emitCol };

                    if (ao0 + ao2 > ao1 + ao3) {
                        vertices.push_back(vt0); vertices.push_back(vt1); vertices.push_back(vt2);
                        vertices.push_back(vt0); vertices.push_back(vt2); vertices.push_back(vt3);
                    } else {
                        vertices.push_back(vt1); vertices.push_back(vt2); vertices.push_back(vt3);
                        vertices.push_back(vt1); vertices.push_back(vt3); vertices.push_back(vt0);
                    }
                }

                // --- 5. EAST FACE (+X) ---
                if (isTransparentBlock(x + 1, y, z)) {
                    atlas.getTileUVs(data.textures.right, u0, v0, u1, v1);
                    glm::vec3 norm(1.0f, 0.0f, 0.0f);

                    float ao0 = (float)calculateVertexAO(isSolidBlock(x + 1, y, z + 1), isSolidBlock(x + 1, y - 1, z), isSolidBlock(x + 1, y - 1, z + 1));
                    float ao1 = (float)calculateVertexAO(isSolidBlock(x + 1, y, z - 1), isSolidBlock(x + 1, y - 1, z), isSolidBlock(x + 1, y - 1, z - 1));
                    float ao2 = (float)calculateVertexAO(isSolidBlock(x + 1, y, z - 1), isSolidBlock(x + 1, y + 1, z), isSolidBlock(x + 1, y + 1, z - 1));
                    float ao3 = (float)calculateVertexAO(isSolidBlock(x + 1, y, z + 1), isSolidBlock(x + 1, y + 1, z), isSolidBlock(x + 1, y + 1, z + 1));

                    VoxelVertex vt0{ {fx + 1, fy,     fz + 1}, {u0, v1}, norm, ao0, emissive, emitCol };
                    VoxelVertex vt1{ {fx + 1, fy,     fz    }, {u1, v1}, norm, ao1, emissive, emitCol };
                    VoxelVertex vt2{ {fx + 1, fy + 1, fz    }, {u1, v0}, norm, ao2, emissive, emitCol };
                    VoxelVertex vt3{ {fx + 1, fy + 1, fz + 1}, {u0, v0}, norm, ao3, emissive, emitCol };

                    if (ao0 + ao2 > ao1 + ao3) {
                        vertices.push_back(vt0); vertices.push_back(vt1); vertices.push_back(vt2);
                        vertices.push_back(vt0); vertices.push_back(vt2); vertices.push_back(vt3);
                    } else {
                        vertices.push_back(vt1); vertices.push_back(vt2); vertices.push_back(vt3);
                        vertices.push_back(vt1); vertices.push_back(vt3); vertices.push_back(vt0);
                    }
                }

                // --- 6. WEST FACE (-X) ---
                if (isTransparentBlock(x - 1, y, z)) {
                    atlas.getTileUVs(data.textures.left, u0, v0, u1, v1);
                    glm::vec3 norm(-1.0f, 0.0f, 0.0f);

                    float ao0 = (float)calculateVertexAO(isSolidBlock(x - 1, y, z - 1), isSolidBlock(x - 1, y - 1, z), isSolidBlock(x - 1, y - 1, z - 1));
                    float ao1 = (float)calculateVertexAO(isSolidBlock(x - 1, y, z + 1), isSolidBlock(x - 1, y - 1, z), isSolidBlock(x - 1, y - 1, z + 1));
                    float ao2 = (float)calculateVertexAO(isSolidBlock(x - 1, y, z + 1), isSolidBlock(x - 1, y + 1, z), isSolidBlock(x - 1, y + 1, z + 1));
                    float ao3 = (float)calculateVertexAO(isSolidBlock(x - 1, y, z - 1), isSolidBlock(x - 1, y + 1, z), isSolidBlock(x - 1, y + 1, z - 1));

                    VoxelVertex vt0{ {fx, fy,     fz    }, {u0, v1}, norm, ao0, emissive, emitCol };
                    VoxelVertex vt1{ {fx, fy,     fz + 1}, {u1, v1}, norm, ao1, emissive, emitCol };
                    VoxelVertex vt2{ {fx, fy + 1, fz + 1}, {u1, v0}, norm, ao2, emissive, emitCol };
                    VoxelVertex vt3{ {fx, fy + 1, fz    }, {u0, v0}, norm, ao3, emissive, emitCol };

                    if (ao0 + ao2 > ao1 + ao3) {
                        vertices.push_back(vt0); vertices.push_back(vt1); vertices.push_back(vt2);
                        vertices.push_back(vt0); vertices.push_back(vt2); vertices.push_back(vt3);
                    } else {
                        vertices.push_back(vt1); vertices.push_back(vt2); vertices.push_back(vt3);
                        vertices.push_back(vt1); vertices.push_back(vt3); vertices.push_back(vt0);
                    }
                }
            }
        }
    }

    return vertices;
}
