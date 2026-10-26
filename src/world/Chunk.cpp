#include "Chunk.h"
#include "ChunkMesher.h"
#include "World.h"
#include <cstring>

Chunk::Chunk(int cx, int cz) : chunkX(cx), chunkZ(cz) {
    std::memset(m_blocks, BLOCK_AIR, sizeof(m_blocks));
}

Chunk::~Chunk() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }
}

BlockId Chunk::getBlock(int lx, int ly, int lz) const {
    if (!inBounds(lx, ly, lz)) return BLOCK_AIR;
    return static_cast<BlockId>(m_blocks[lx][ly][lz]);
}

void Chunk::setBlock(int lx, int ly, int lz, BlockId id) {
    if (!inBounds(lx, ly, lz)) return;
    m_blocks[lx][ly][lz] = static_cast<uint8_t>(id);
    isDirty = true;
}

void Chunk::buildMesh(const World& world) {
    std::vector<VoxelVertex> vertices = ChunkMesher::generateMesh(*this, world, world.getTextureAtlas());
    m_vertexCount = static_cast<int>(vertices.size());

    if (m_vao == 0) {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, position));

        // TexCoord
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, texCoord));

        // Normal
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, normal));

        // AO
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, ao));

        // Emissive
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, emissive));

        // EmitColor
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, emitColor));

        glBindVertexArray(0);
    }

    if (m_vertexCount > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelVertex), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    isDirty = false;
}

void Chunk::render() const {
    if (m_vertexCount == 0 || m_vao == 0) return;
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);
}
