#include "Raycast.h"
#include <cmath>

RaycastResult Raycast::cast(const World& world, const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    RaycastResult result;

    glm::vec3 dir = glm::normalize(direction);

    // Current voxel
    int x = (int)std::floor(origin.x);
    int y = (int)std::floor(origin.y);
    int z = (int)std::floor(origin.z);

    int stepX = (dir.x > 0) ? 1 : ((dir.x < 0) ? -1 : 0);
    int stepY = (dir.y > 0) ? 1 : ((dir.y < 0) ? -1 : 0);
    int stepZ = (dir.z > 0) ? 1 : ((dir.z < 0) ? -1 : 0);

    float deltaX = (stepX != 0) ? std::abs(1.0f / dir.x) : 1e30f;
    float deltaY = (stepY != 0) ? std::abs(1.0f / dir.y) : 1e30f;
    float deltaZ = (stepZ != 0) ? std::abs(1.0f / dir.z) : 1e30f;

    float sideDistX = (stepX > 0) ? (x + 1.0f - origin.x) * deltaX : (origin.x - x) * deltaX;
    float sideDistY = (stepY > 0) ? (y + 1.0f - origin.y) * deltaY : (origin.y - y) * deltaY;
    float sideDistZ = (stepZ > 0) ? (z + 1.0f - origin.z) * deltaZ : (origin.z - z) * deltaZ;

    glm::ivec3 lastNormal(0);
    float currentDist = 0.0f;

    while (currentDist <= maxDistance) {
        BlockId b = world.getBlock(x, y, z);
        if (b != BLOCK_AIR && b != BLOCK_WATER) {
            result.hit = true;
            result.blockPos = glm::ivec3(x, y, z);
            result.normal = lastNormal;
            result.placePos = result.blockPos + result.normal;
            result.blockId = b;
            result.distance = currentDist;
            return result;
        }

        if (sideDistX < sideDistY) {
            if (sideDistX < sideDistZ) {
                currentDist = sideDistX;
                sideDistX += deltaX;
                x += stepX;
                lastNormal = glm::ivec3(-stepX, 0, 0);
            } else {
                currentDist = sideDistZ;
                sideDistZ += deltaZ;
                z += stepZ;
                lastNormal = glm::ivec3(0, 0, -stepZ);
            }
        } else {
            if (sideDistY < sideDistZ) {
                currentDist = sideDistY;
                sideDistY += deltaY;
                y += stepY;
                lastNormal = glm::ivec3(0, -stepY, 0);
            } else {
                currentDist = sideDistZ;
                sideDistZ += deltaZ;
                z += stepZ;
                lastNormal = glm::ivec3(0, 0, -stepZ);
            }
        }
    }

    return result;
}
