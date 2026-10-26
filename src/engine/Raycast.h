#pragma once
#include "Common.h"
#include "../world/World.h"

struct RaycastResult {
    bool hit = false;
    glm::ivec3 blockPos{0, 0, 0};
    glm::ivec3 normal{0, 0, 0};
    glm::ivec3 placePos{0, 0, 0};
    BlockId blockId = BLOCK_AIR;
    float distance = 0.0f;
};

class Raycast {
public:
    static RaycastResult cast(const World& world, const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 7.5f);
};
