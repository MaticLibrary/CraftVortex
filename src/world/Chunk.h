#pragma once
#include "../engine/Common.h"
#include "Block.h"

struct VoxelVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    float ao;
    float emissive;
    glm::vec3 emitColor;
};

class World;

class Chunk {
public:
    int chunkX;
    int chunkZ;

    bool isDirty = true;
    bool isGenerated = false;

    Chunk(int cx, int cz);
    ~Chunk();

    BlockId getBlock(int lx, int ly, int lz) const;
    void setBlock(int lx, int ly, int lz, BlockId id);

    // Fast inline access
    static bool inBounds(int lx, int ly, int lz) {
        return lx >= 0 && lx < CHUNK_SIZE_X &&
               ly >= 0 && ly < CHUNK_HEIGHT &&
               lz >= 0 && lz < CHUNK_SIZE_Z;
    }

    void buildMesh(const World& world);
    void render() const;

    int getVertexCount() const { return m_vertexCount; }

private:
    uint8_t m_blocks[CHUNK_SIZE_X][CHUNK_HEIGHT][CHUNK_SIZE_Z];

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    int m_vertexCount = 0;
};
