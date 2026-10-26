#include "BlockParticles.h"

namespace {
void appendFace(std::vector<VoxelVertex>& vertices,
                const glm::vec3& a, const glm::vec3& b,
                const glm::vec3& c, const glm::vec3& d,
                const glm::vec3& normal,
                float u0, float v0, float u1, float v1) {
    const VoxelVertex vA{a, {u0, v1}, normal, 3.0f, 0.0f, glm::vec3(0.0f)};
    const VoxelVertex vB{b, {u1, v1}, normal, 3.0f, 0.0f, glm::vec3(0.0f)};
    const VoxelVertex vC{c, {u1, v0}, normal, 3.0f, 0.0f, glm::vec3(0.0f)};
    const VoxelVertex vD{d, {u0, v0}, normal, 3.0f, 0.0f, glm::vec3(0.0f)};
    vertices.insert(vertices.end(), {vA, vB, vC, vA, vC, vD});
}
}

BlockParticles::~BlockParticles() {
    for (Mesh& mesh : m_blockMeshes) {
        if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
        if (mesh.vbo) glDeleteBuffers(1, &mesh.vbo);
    }
}

void BlockParticles::init(const TextureAtlas& atlas) {
    for (int id = 1; id < BLOCK_COUNT; ++id) {
        const BlockData& data = BlockRegistry::get(static_cast<BlockId>(id));
        float u0, v0, u1, v1;
        atlas.getTileUVs(data.textures.top, u0, v0, u1, v1);

        std::vector<VoxelVertex> vertices;
        vertices.reserve(36);
        const glm::vec3 p000(0, 0, 0), p001(0, 0, 1), p010(0, 1, 0), p011(0, 1, 1);
        const glm::vec3 p100(1, 0, 0), p101(1, 0, 1), p110(1, 1, 0), p111(1, 1, 1);
        appendFace(vertices, p100, p110, p111, p101, { 1, 0, 0}, u0, v0, u1, v1);
        appendFace(vertices, p000, p001, p011, p010, {-1, 0, 0}, u0, v0, u1, v1);
        appendFace(vertices, p010, p011, p111, p110, { 0, 1, 0}, u0, v0, u1, v1);
        appendFace(vertices, p000, p100, p101, p001, { 0,-1, 0}, u0, v0, u1, v1);
        appendFace(vertices, p001, p101, p111, p011, { 0, 0, 1}, u0, v0, u1, v1);
        appendFace(vertices, p000, p010, p110, p100, { 0, 0,-1}, u0, v0, u1, v1);

        Mesh& mesh = m_blockMeshes[id];
        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);
        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelVertex), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, texCoord));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, normal));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, ao));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, emissive));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, emitColor));
    }
    glBindVertexArray(0);
}

float BlockParticles::random01() {
    m_randomState ^= m_randomState << 13;
    m_randomState ^= m_randomState >> 17;
    m_randomState ^= m_randomState << 5;
    return static_cast<float>(m_randomState & 0x00ffffffu) / 16777215.0f;
}

void BlockParticles::spawn(const glm::ivec3& blockPosition, BlockId blockId) {
    if (blockId <= BLOCK_AIR || blockId >= BLOCK_COUNT) return;
    constexpr int fragmentCount = 9;
    for (int i = 0; i < fragmentCount; ++i) {
        Particle particle;
        particle.position = glm::vec3(blockPosition) + glm::vec3(0.5f) +
                           glm::vec3(random01() - 0.5f, random01() - 0.5f, random01() - 0.5f) * 0.55f;
        glm::vec3 direction(random01() * 2.0f - 1.0f,
                            0.25f + random01() * 1.2f,
                            random01() * 2.0f - 1.0f);
        particle.velocity = glm::normalize(direction) * (1.6f + random01() * 3.2f);
        particle.block = blockId;
        particle.lifetime = 0.42f + random01() * 0.48f;
        particle.size = 0.11f + random01() * 0.11f;
        m_particles.push_back(particle);
    }
}

void BlockParticles::update(float dt) {
    for (Particle& particle : m_particles) {
        particle.age += dt;
        particle.velocity.y -= 13.0f * dt;
        particle.position += particle.velocity * dt;
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(), [](const Particle& particle) {
        return particle.age >= particle.lifetime;
    }), m_particles.end());
}

void BlockParticles::render(const Shader& shader, const glm::vec3& playerPosition) {
    for (const Particle& particle : m_particles) {
        glm::vec3 delta = particle.position - playerPosition;
        if (glm::dot(delta, delta) > 90.0f * 90.0f) continue;
        float fade = 1.0f - particle.age / particle.lifetime;
        float size = particle.size * (0.45f + 0.55f * fade);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), particle.position);
        model = glm::scale(model, glm::vec3(size));
        shader.setMat4("uModel", model);
        glBindVertexArray(m_blockMeshes[particle.block].vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}
