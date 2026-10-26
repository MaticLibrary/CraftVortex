#include "MobManager.h"

namespace {
void appendFace(std::vector<VoxelVertex>& vertices, const TextureAtlas& atlas, int tile,
                const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
                const glm::vec3& normal, float emissive = 0.0f, const glm::vec3& emitColor = glm::vec3(0.0f)) {
    float u0, v0, u1, v1;
    atlas.getTileUVs(tile, u0, v0, u1, v1);
    VoxelVertex a{p0, {u0, v1}, normal, 3.0f, emissive, emitColor};
    VoxelVertex b{p1, {u1, v1}, normal, 3.0f, emissive, emitColor};
    VoxelVertex c{p2, {u1, v0}, normal, 3.0f, emissive, emitColor};
    VoxelVertex d{p3, {u0, v0}, normal, 3.0f, emissive, emitColor};
    vertices.insert(vertices.end(), {a, b, c, a, c, d});
}

void appendBox(std::vector<VoxelVertex>& vertices, const TextureAtlas& atlas, int tile,
               glm::vec3 lo, glm::vec3 hi, float emissive = 0.0f, glm::vec3 emitColor = glm::vec3(0.0f)) {
    appendFace(vertices, atlas, tile, {lo.x,lo.y,hi.z}, {hi.x,lo.y,hi.z}, {hi.x,hi.y,hi.z}, {lo.x,hi.y,hi.z}, {0,0,1}, emissive, emitColor);
    appendFace(vertices, atlas, tile, {hi.x,lo.y,lo.z}, {lo.x,lo.y,lo.z}, {lo.x,hi.y,lo.z}, {hi.x,hi.y,lo.z}, {0,0,-1}, emissive, emitColor);
    appendFace(vertices, atlas, tile, {lo.x,hi.y,hi.z}, {hi.x,hi.y,hi.z}, {hi.x,hi.y,lo.z}, {lo.x,hi.y,lo.z}, {0,1,0}, emissive, emitColor);
    appendFace(vertices, atlas, tile, {lo.x,lo.y,lo.z}, {hi.x,lo.y,lo.z}, {hi.x,lo.y,hi.z}, {lo.x,lo.y,hi.z}, {0,-1,0}, emissive, emitColor);
    appendFace(vertices, atlas, tile, {hi.x,lo.y,hi.z}, {hi.x,lo.y,lo.z}, {hi.x,hi.y,lo.z}, {hi.x,hi.y,hi.z}, {1,0,0}, emissive, emitColor);
    appendFace(vertices, atlas, tile, {lo.x,lo.y,lo.z}, {lo.x,lo.y,hi.z}, {lo.x,hi.y,hi.z}, {lo.x,hi.y,lo.z}, {-1,0,0}, emissive, emitColor);
}

int floorToInt(float v) { return static_cast<int>(std::floor(v)); }
}

MobManager::~MobManager() {
    for (Mesh& mesh : m_meshes) {
        if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
        if (mesh.vbo) glDeleteBuffers(1, &mesh.vbo);
    }
}

uint32_t MobManager::hash(int x, int z, uint32_t salt) {
    uint32_t h = static_cast<uint32_t>(x) * 0x8da6b343u;
    h ^= static_cast<uint32_t>(z) * 0xd8163841u;
    h ^= salt + 0xcb1ab31fu;
    h ^= h >> 13;
    h *= 0x85ebca6bu;
    return h ^ (h >> 16);
}

bool MobManager::isHostile(MobKind kind) {
    return kind == MobKind::Elf || kind == MobKind::Orc;
}

glm::vec3 MobManager::colorFor(MobKind kind) {
    switch (kind) {
        case MobKind::Elf: return glm::vec3(0.35f, 0.95f, 0.55f);
        case MobKind::Orc: return glm::vec3(0.75f, 0.2f, 0.1f);
        case MobKind::Fairy: return glm::vec3(0.25f, 0.95f, 1.0f);
        default: return glm::vec3(0.0f);
    }
}

void MobManager::init(const TextureAtlas& atlas) {
    for (int i = 0; i < 4; ++i) buildMesh(static_cast<MobKind>(i), atlas);
}

void MobManager::buildMesh(MobKind kind, const TextureAtlas& atlas) {
    std::vector<VoxelVertex> vertices;
    const int id = static_cast<int>(kind);
    int skin = kind == MobKind::Orc ? 21 : (kind == MobKind::Fairy ? 8 : 19);
    int cloth = kind == MobKind::Elf ? 14 : (kind == MobKind::Orc ? 5 : (kind == MobKind::Fairy ? 7 : 15));
    int legs = kind == MobKind::Orc ? 16 : 12;

    float width = kind == MobKind::Fairy ? 0.18f : 0.26f;
    float headSize = kind == MobKind::Fairy ? 0.22f : 0.30f;
    float bodyTop = kind == MobKind::Fairy ? 0.95f : 1.25f;
    float bodyBottom = kind == MobKind::Fairy ? 0.48f : 0.55f;

    appendBox(vertices, atlas, cloth, {-width, bodyBottom, -0.16f}, {width, bodyTop, 0.16f},
              kind == MobKind::Fairy ? 0.45f : 0.0f, colorFor(kind));
    appendBox(vertices, atlas, skin, {-headSize, bodyTop + 0.08f, -headSize},
              {headSize, bodyTop + 0.08f + headSize * 2.0f, headSize},
              kind == MobKind::Fairy ? 0.5f : 0.0f, colorFor(kind));

    float legTop = bodyBottom;
    float legBottom = kind == MobKind::Fairy ? 0.25f : 0.0f;
    appendBox(vertices, atlas, legs, {-0.23f, legBottom, -0.12f}, {-0.04f, legTop, 0.12f});
    appendBox(vertices, atlas, legs, {0.04f, legBottom, -0.12f}, {0.23f, legTop, 0.12f});

    // Arms, pointed ears, and fairy wings give each silhouette its own read.
    appendBox(vertices, atlas, skin, {-0.42f, bodyBottom + 0.05f, -0.11f}, {-width, bodyTop - 0.05f, 0.11f});
    appendBox(vertices, atlas, skin, {width, bodyBottom + 0.05f, -0.11f}, {0.42f, bodyTop - 0.05f, 0.11f});
    if (kind == MobKind::Elf) {
        appendBox(vertices, atlas, cloth, {-0.43f, bodyTop + 0.18f, -0.035f}, {-0.24f, bodyTop + 0.25f, 0.035f});
        appendBox(vertices, atlas, cloth, {0.24f, bodyTop + 0.18f, -0.035f}, {0.43f, bodyTop + 0.25f, 0.035f});
    } else if (kind == MobKind::Orc) {
        appendBox(vertices, atlas, 10, {-0.14f, bodyTop + 0.08f, 0.23f}, {-0.06f, bodyTop + 0.18f, 0.30f});
        appendBox(vertices, atlas, 10, {0.06f, bodyTop + 0.08f, 0.23f}, {0.14f, bodyTop + 0.18f, 0.30f});
    } else if (kind == MobKind::Fairy) {
        appendBox(vertices, atlas, 7, {-0.62f, 0.60f, -0.06f}, {-0.22f, 1.0f, 0.0f}, 0.8f, colorFor(kind));
        appendBox(vertices, atlas, 7, {0.22f, 0.60f, -0.06f}, {0.62f, 1.0f, 0.0f}, 0.8f, colorFor(kind));
    }

    Mesh& mesh = m_meshes[id];
    mesh.vertexCount = static_cast<GLsizei>(vertices.size());
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
    glBindVertexArray(0);
}

void MobManager::update(float dt, const World& world, Player& player) {
    m_time += dt;
    m_playerAttackCooldown = std::max(0.0f, m_playerAttackCooldown - dt);

    constexpr int cellSize = 52;
    int playerCellX = floorToInt(player.position.x / cellSize);
    int playerCellZ = floorToInt(player.position.z / cellSize);
    for (int dz = -2; dz <= 2; ++dz) {
        for (int dx = -2; dx <= 2; ++dx) {
            int cellX = playerCellX + dx;
            int cellZ = playerCellZ + dz;
            uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(cellX)) << 32) |
                           static_cast<uint32_t>(cellZ);
            if (m_mobs.find(key) != m_mobs.end()) continue;

            uint32_t h = hash(cellX, cellZ, 0x4d4f4253u);
            int wx = cellX * cellSize + static_cast<int>(h % cellSize);
            int wz = cellZ * cellSize + static_cast<int>((h >> 8) % cellSize);
            float dxp = wx + 0.5f - player.position.x;
            float dzp = wz + 0.5f - player.position.z;
            if (dxp * dxp + dzp * dzp > 112.0f * 112.0f) continue;
            int cx = floorToInt((float)wx / CHUNK_SIZE_X);
            int cz = floorToInt((float)wz / CHUNK_SIZE_Z);
            if (!world.getChunk(cx, cz)) continue;

            int ground = world.getSpawnHeight(wx, wz);
            if (ground <= SEA_LEVEL + 1 || ground >= CHUNK_HEIGHT - 8) continue;
            Mob mob;
            int choice = static_cast<int>((h >> 16) % 100);
            if (choice < 29) mob.kind = MobKind::Elf;
            else if (choice < 57) mob.kind = MobKind::Orc;
            else if (choice < 84) mob.kind = MobKind::Human;
            else mob.kind = MobKind::Fairy;
            mob.position = glm::vec3(wx + 0.5f, (float)ground + 1.0f, wz + 0.5f);
            mob.wanderTarget = mob.position;
            mob.health = mob.kind == MobKind::Orc ? 48.0f : 30.0f;
            mob.randomState = h | 1u;
            mob.wanderTimer = (h % 300) / 100.0f;
            m_mobs.emplace(key, mob);
        }
    }

    for (auto it = m_mobs.begin(); it != m_mobs.end();) {
        Mob& mob = it->second;
        glm::vec3 toPlayer = player.position - mob.position;
        float distance2 = toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z;
        if (distance2 > 150.0f * 150.0f) {
            it = m_mobs.erase(it);
            continue;
        }
        if (mob.health <= 0.0f) {
            ++it;
            continue;
        }

        mob.attackTimer = std::max(0.0f, mob.attackTimer - dt);
        mob.wanderTimer -= dt;
        glm::vec3 target = mob.wanderTarget;
        if (isHostile(mob.kind) && distance2 < 22.0f * 22.0f && std::abs(toPlayer.y) < 12.0f) target = player.position;
        else if (mob.wanderTimer <= 0.0f) {
            mob.randomState ^= mob.randomState << 13;
            mob.randomState ^= mob.randomState >> 17;
            mob.randomState ^= mob.randomState << 5;
            float angle = (mob.randomState % 6283) / 1000.0f;
            float radius = 3.0f + ((mob.randomState >> 10) % 800) / 100.0f;
            mob.wanderTarget = mob.position + glm::vec3(std::cos(angle) * radius, 0.0f, std::sin(angle) * radius);
            mob.wanderTimer = 2.0f + ((mob.randomState >> 20) % 30) / 10.0f;
            target = mob.wanderTarget;
        }

        glm::vec3 delta = target - mob.position;
        float horizontalLength = std::sqrt(delta.x * delta.x + delta.z * delta.z);
        if (isHostile(mob.kind) && distance2 < 2.25f && std::abs(toPlayer.y) < 2.2f && mob.attackTimer <= 0.0f) {
            player.takeDamage(mob.kind == MobKind::Orc ? 14.0f : 9.0f);
            mob.attackTimer = mob.kind == MobKind::Orc ? 1.5f : 1.0f;
        }

        if (horizontalLength > 0.35f) {
            float speed = mob.kind == MobKind::Orc ? 1.9f : (mob.kind == MobKind::Fairy ? 3.4f : 2.5f);
            glm::vec3 step(delta.x / horizontalLength * speed * dt, 0.0f,
                           delta.z / horizontalLength * speed * dt);
            int nextX = floorToInt(mob.position.x + step.x);
            int nextZ = floorToInt(mob.position.z + step.z);
            int nextY = world.getSpawnHeight(nextX, nextZ) + 1;
            int currentY = world.getSpawnHeight(floorToInt(mob.position.x), floorToInt(mob.position.z)) + 1;
            bool clear = std::abs(nextY - currentY) <= 2;
            clear = clear && !BlockRegistry::get(world.getBlock(nextX, nextY, nextZ)).isSolid;
            clear = clear && !BlockRegistry::get(world.getBlock(nextX, nextY + 1, nextZ)).isSolid;
            if (clear) {
                mob.position.x += step.x;
                mob.position.z += step.z;
                mob.position.y += ((float)nextY - mob.position.y) * std::min(1.0f, dt * 5.0f);
            } else {
                mob.wanderTimer = 0.0f;
            }
        }
        ++it;
    }
}

bool MobManager::tryHit(const glm::vec3& origin, const glm::vec3& direction, float damage,
                        float maxRange, const World* world) {
    if (m_playerAttackCooldown > 0.0f) return false;
    glm::vec3 dir = glm::normalize(direction);
    auto target = m_mobs.end();
    float closest = maxRange;
    if (world) {
        RaycastResult obstruction = Raycast::cast(*world, origin, dir, maxRange);
        if (obstruction.hit) closest = std::min(closest, obstruction.distance + 0.15f);
    }
    for (auto it = m_mobs.begin(); it != m_mobs.end(); ++it) {
        if (it->second.health <= 0.0f) continue;
        glm::vec3 center = it->second.position + glm::vec3(0.0f, 1.05f, 0.0f);
        glm::vec3 toMob = center - origin;
        float along = glm::dot(toMob, dir);
        if (along < 0.0f || along > closest) continue;
        glm::vec3 nearest = origin + dir * along;
        glm::vec3 offset = center - nearest;
        if (glm::length(offset) < 0.85f) {
            target = it;
            closest = along;
        }
    }
    if (target == m_mobs.end()) return false;
    target->second.health -= damage;
    m_playerAttackCooldown = 0.42f;
    if (target->second.health <= 0.0f) target->second.health = 0.0f;
    return true;
}

void MobManager::render(const Shader& shader, const TextureAtlas& atlas, const glm::vec3& playerPos) {
    atlas.bind(0);
    for (const auto& entry : m_mobs) {
        const Mob& mob = entry.second;
        if (mob.health <= 0.0f) continue;
        glm::vec3 diff = mob.position - playerPos;
        if (diff.x * diff.x + diff.z * diff.z > 145.0f * 145.0f) continue;
        float bob = mob.kind == MobKind::Fairy ? std::sin(m_time * 4.0f + mob.position.x) * 0.12f :
                    std::abs(std::sin(m_time * 7.0f + mob.position.x)) * 0.025f;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), mob.position + glm::vec3(0.0f, bob, 0.0f));
        shader.setMat4("uModel", model);
        const Mesh& mesh = m_meshes[static_cast<int>(mob.kind)];
        glBindVertexArray(mesh.vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    }
    glBindVertexArray(0);
}
