#include "Player.h"

Player::Player(glm::vec3 spawnPos)
    : position(spawnPos),
      camera(spawnPos + glm::vec3(0.0f, 1.62f, 0.0f)),
      m_spawnPosition(spawnPos) {
    for (BlockId id : hotbar) inventory[id] = 32;
    inventory[BLOCK_DARK_WOOD] = 8;
    inventory[BLOCK_SAND] = 8;
    inventory[BLOCK_BASALT] = 4;
    inventory[BLOCK_OBSIDIAN] = 4;
}

CraftRecipe Player::getCraftRecipe(int index) {
    static const CraftRecipe recipes[CRAFT_RECIPE_COUNT] = {
        { BLOCK_DARK_WOOD, 1, BLOCK_AIR, 0, BLOCK_WOOD_PLANKS, 4 },
        { BLOCK_STONE, 2, BLOCK_AMETHYST, 1, BLOCK_RUNE_STONE, 1 },
        { BLOCK_SAND, 2, BLOCK_STONE, 1, BLOCK_SANDSTONE, 4 },
        { BLOCK_BASALT, 2, BLOCK_OBSIDIAN, 1, BLOCK_GLOW_LANTERN, 1 },
        { BLOCK_WOOD_PLANKS, 3, BLOCK_AIR, 0, BLOCK_BOOKSHELF, 1 },
        { BLOCK_WOOD_PLANKS, 4, BLOCK_GOLD_ORE, 1, BLOCK_CHEST, 1 },
        { BLOCK_STONE, 1, BLOCK_GOLD_ORE, 1, BLOCK_IRON_CHAIN, 3 },
        { BLOCK_STONE, 4, BLOCK_AIR, 0, BLOCK_MARBLE, 4 },
        { BLOCK_RUNE_STONE, 1, BLOCK_AMETHYST, 1, BLOCK_CRYSTAL_BRICK, 2 },
        { BLOCK_WOOD_PLANKS, 2, BLOCK_STONE, 1, BLOCK_AIR, 1, CraftOutputType::Tool, ToolType::Axe },
        { BLOCK_WOOD_PLANKS, 1, BLOCK_STONE, 2, BLOCK_AIR, 1, CraftOutputType::Tool, ToolType::Sword },
        { BLOCK_WOOD_PLANKS, 3, BLOCK_AMETHYST, 1, BLOCK_AIR, 1, CraftOutputType::Tool, ToolType::Bow },
        { BLOCK_WOOD_PLANKS, 1, BLOCK_STONE, 1, BLOCK_AIR, 8, CraftOutputType::Arrows }
    };
    if (index < 0 || index >= CRAFT_RECIPE_COUNT) return { BLOCK_AIR, 0, BLOCK_AIR, 0, BLOCK_AIR, 0 };
    return recipes[index];
}

bool Player::canCraftRecipe(int recipeIndex) const {
    CraftRecipe recipe = getCraftRecipe(recipeIndex);
    if (inventory[recipe.inputA] < recipe.amountA) return false;
    if (recipe.amountB > 0 && inventory[recipe.inputB] < recipe.amountB) return false;
    if (recipe.outputType == CraftOutputType::Tool)
        return !ownedTools[static_cast<int>(recipe.toolOutput)];
    if (recipe.outputType == CraftOutputType::Arrows) return arrows + recipe.outputAmount <= 999;
    return recipe.output != BLOCK_AIR && inventory[recipe.output] + recipe.outputAmount <= 999;
}

bool Player::craftRecipe(int recipeIndex) {
    if (!canCraftRecipe(recipeIndex)) return false;
    CraftRecipe recipe = getCraftRecipe(recipeIndex);
    inventory[recipe.inputA] -= recipe.amountA;
    if (recipe.amountB > 0) inventory[recipe.inputB] -= recipe.amountB;
    if (recipe.outputType == CraftOutputType::Tool) {
        ownedTools[static_cast<int>(recipe.toolOutput)] = true;
        selectedTool = recipe.toolOutput;
    } else if (recipe.outputType == CraftOutputType::Arrows)
        arrows += recipe.outputAmount;
    else
        inventory[recipe.output] += recipe.outputAmount;
    return true;
}

bool Player::openChestAt(const glm::ivec3& blockPosition, const World& world) {
    if (world.getBlock(blockPosition.x, blockPosition.y, blockPosition.z) != BLOCK_CHEST) return false;
    openChestPosition = blockPosition;
    inventoryOpen = true;
    chestOpen = true;
    craftingOpen = false;
    return true;
}

void Player::closeMenus() {
    inventoryOpen = false;
    chestOpen = false;
    craftingOpen = false;
}

void Player::cycleTool() {
    int current = static_cast<int>(selectedTool);
    for (int step = 1; step <= 4; ++step) {
        int next = (current + step) % 4;
        if (ownedTools[next]) {
            selectedTool = static_cast<ToolType>(next);
            return;
        }
    }
}

void Player::takeDamage(float damage) {
    if (m_attackCooldown > 0.0f) return;
    health = std::max(0.0f, health - damage);
    m_attackCooldown = 0.8f;
    m_damageFlash = 0.35f;
    if (health <= 0.0f) {
        health = 100.0f;
        position = m_spawnPosition;
        velocity = glm::vec3(0.0f);
        isFlying = false;
    }
}

const char* Player::getToolName() const {
    switch (selectedTool) {
        case ToolType::Axe: return "AXE";
        case ToolType::Sword: return "SWORD";
        case ToolType::Bow: return "BOW";
        default: return "PICKAXE";
    }
}

float Player::getAttackDamage() const {
    switch (selectedTool) {
        case ToolType::Axe: return 17.0f;
        case ToolType::Sword: return 30.0f;
        case ToolType::Bow: return 28.0f;
        default: return 12.0f;
    }
}

RaycastResult Player::getLookTarget(const World& world) const {
    glm::vec3 eyePos = position + glm::vec3(0.0f, m_eyeHeight, 0.0f);
    return Raycast::cast(world, eyePos, camera.front, 7.5f);
}

void Player::handleScroll(double yoffset) {
    if (yoffset > 0) {
        selectedSlot = (selectedSlot - 1 + HOTBAR_SLOTS) % HOTBAR_SLOTS;
    } else if (yoffset < 0) {
        selectedSlot = (selectedSlot + 1) % HOTBAR_SLOTS;
    }
}

void Player::handleMouseClick(int button, int action, World& world) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        m_miningActive = false;
        m_miningProgress = 0.0f;
        return;
    }
    if (action != GLFW_PRESS || craftingOpen || inventoryOpen || chestOpen) return;

    RaycastResult hit = getLookTarget(world);

    if (button == GLFW_MOUSE_BUTTON_LEFT && selectedTool != ToolType::Bow) {
        if (hit.hit && hit.blockPos.y > 0) {
            m_miningActive = true;
            m_miningBlock = hit.blockPos;
            m_miningProgress = 0.0f;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (hit.hit) {
            glm::ivec3 p = hit.placePos;
            AABB blockBox{ glm::vec3(p.x, p.y, p.z), glm::vec3(p.x + 1.0f, p.y + 1.0f, p.z + 1.0f) };
            BlockId selected = getSelectedBlock();
            bool canPlace = isFlying || inventory[selected] > 0;
            if (world.getBlock(p.x, p.y, p.z) == BLOCK_AIR && !getAABB().intersects(blockBox) && canPlace) {
                world.setBlock(p.x, p.y, p.z, selected);
                if (!isFlying) --inventory[selected];
            }
        }
    }
}

void Player::updateMining(float dt, World& world) {
    if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS || !m_miningActive ||
        craftingOpen || inventoryOpen || chestOpen || selectedTool == ToolType::Bow) {
        m_miningActive = false;
        m_miningProgress = 0.0f;
        return;
    }

    RaycastResult hit = getLookTarget(world);
    if (!hit.hit || hit.blockPos.x != m_miningBlock.x || hit.blockPos.y != m_miningBlock.y ||
        hit.blockPos.z != m_miningBlock.z || hit.blockPos.y <= 0) {
        m_miningActive = false;
        m_miningProgress = 0.0f;
        return;
    }

    bool isStone = hit.blockId == BLOCK_STONE || hit.blockId == BLOCK_BASALT || hit.blockId == BLOCK_OBSIDIAN ||
                   hit.blockId == BLOCK_COBBLESTONE || hit.blockId == BLOCK_DUNGEON_BRICK ||
                   hit.blockId == BLOCK_MARBLE || hit.blockId == BLOCK_GOLD_TRIM || hit.blockId == BLOCK_CRYSTAL_BRICK ||
                   hit.blockId == BLOCK_IRON_CHAIN;
    bool isWood = hit.blockId == BLOCK_DARK_WOOD || hit.blockId == BLOCK_WOOD_PLANKS ||
                  hit.blockId == BLOCK_BOOKSHELF || hit.blockId == BLOCK_CHEST;
    float miningTime = 0.52f;
    if (isStone) miningTime = selectedTool == ToolType::Pickaxe ? 0.52f : 1.45f;
    else if (isWood) miningTime = selectedTool == ToolType::Axe ? 0.35f : 0.95f;
    else if (hit.blockId == BLOCK_SAND || hit.blockId == BLOCK_SNOW || hit.blockId == BLOCK_MARSH_MUD)
        miningTime = 0.38f;
    else if (selectedTool == ToolType::Sword) miningTime = 0.9f;

    m_miningProgress += dt / miningTime;
    if (m_miningProgress >= 1.0f) {
        if (hit.blockId == BLOCK_CHEST) {
            if (ChestInventory* chest = world.getChestInventory(hit.blockPos)) {
                for (int id = 1; id < BLOCK_COUNT; ++id)
                    inventory[id] = std::min(999, inventory[id] + chest->blocks[id]);
                arrows = std::min(999, arrows + chest->arrows);
            }
        }
        world.setBlock(hit.blockPos.x, hit.blockPos.y, hit.blockPos.z, BLOCK_AIR);
        if (!isFlying) inventory[hit.blockId] = std::min(999, inventory[hit.blockId] + 1);
        m_lastBrokenBlock = hit.blockPos;
        m_lastBrokenBlockId = hit.blockId;
        m_hasBreakEvent = true;
        m_miningProgress = 0.0f;
        m_miningActive = false;
    }
}

void Player::update(float dt, World& world, GLFWwindow* window) {
    m_attackCooldown = std::max(0.0f, m_attackCooldown - dt);
    m_damageFlash = std::max(0.0f, m_damageFlash - dt);
    m_hitConfirmTimer = std::max(0.0f, m_hitConfirmTimer - dt);
    updateMining(dt, world);
    if (craftingOpen || inventoryOpen || chestOpen) {
        camera.position = position + glm::vec3(0.0f, m_eyeHeight, 0.0f);
        return;
    }
    // 1. Process movement inputs
    glm::vec3 moveDir(0.0f);
    glm::vec3 camFrontFlat = glm::normalize(glm::vec3(camera.front.x, 0.0f, camera.front.z));
    glm::vec3 camRightFlat = glm::normalize(glm::vec3(camera.right.x, 0.0f, camera.right.z));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += camFrontFlat;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= camFrontFlat;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= camRightFlat;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += camRightFlat;

    if (glm::length(moveDir) > 0.001f) {
        moveDir = glm::normalize(moveDir);
    }

    isSprinting = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    bool qDown = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    if (qDown && !m_qKeyLast)
        cycleTool();
    m_qKeyLast = qDown;

    // Number keys 1-9 for slot selection
    for (int k = GLFW_KEY_1; k <= GLFW_KEY_9; ++k) {
        if (glfwGetKey(window, k) == GLFW_PRESS) {
            selectedSlot = k - GLFW_KEY_1;
        }
    }

    // Toggle Fly mode with 'F' key (handled in key callback or edge check)
    static bool fKeyLast = false;
    bool fKeyDown = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
    if (fKeyDown && !fKeyLast) {
        isFlying = !isFlying;
        velocity = glm::vec3(0.0f);
    }
    fKeyLast = fKeyDown;

    // 2. Physics / movement update
    updatePhysics(dt, world, moveDir);

    // 3. Keep camera pinned to player's eye level
    camera.position = position + glm::vec3(0.0f, m_eyeHeight, 0.0f);
}

void Player::updatePhysics(float dt, const World& world, const glm::vec3& inputDir) {
    if (isFlying) {
        // Creative 3D Flight
        float flySpeed = isSprinting ? 35.0f : 16.0f;
        glm::vec3 flyMove = inputDir * flySpeed;

        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
            flyMove.y += flySpeed;
        }
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            flyMove.y -= flySpeed;
        }

        position += flyMove * dt;
        isGrounded = false;
        return;
    }

    // Walking / Survival mode
    float speed = isSprinting ? 8.0f : 5.0f;
    float accel = 45.0f;

    // Horizontal acceleration
    glm::vec2 targetVel(inputDir.x * speed, inputDir.z * speed);
    velocity.x += (targetVel.x - velocity.x) * std::min(1.0f, accel * dt);
    velocity.z += (targetVel.y - velocity.z) * std::min(1.0f, accel * dt);

    // Gravity
    velocity.y -= 26.0f * dt;
    if (velocity.y < -40.0f) velocity.y = -40.0f;

    // Jump
    if (isGrounded && glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS) {
        velocity.y = 8.8f;
        isGrounded = false;
    }

    // Collision detection & resolution with voxel world
    glm::vec3 delta = velocity * dt;
    resolveVoxelCollisions(world, delta);
    position += delta;
}

void Player::resolveVoxelCollisions(const World& world, glm::vec3& moveVec) {
    float halfW = m_width * 0.5f;

    auto testBox = [&](const glm::vec3& pos) -> bool {
        AABB playerBox{
            pos - glm::vec3(halfW, 0.0f, halfW),
            pos + glm::vec3(halfW, m_height, halfW)
        };

        int minX = (int)std::floor(playerBox.min.x);
        int maxX = (int)std::floor(playerBox.max.x);
        int minY = (int)std::floor(playerBox.min.y);
        int maxY = (int)std::floor(playerBox.max.y);
        int minZ = (int)std::floor(playerBox.min.z);
        int maxZ = (int)std::floor(playerBox.max.z);

        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                for (int x = minX; x <= maxX; ++x) {
                    BlockId b = world.getBlock(x, y, z);
                    if (BlockRegistry::get(b).isSolid) {
                        return true;
                    }
                }
            }
        }
        return false;
    };

    // Y Axis collision
    glm::vec3 nextPosY = position + glm::vec3(0.0f, moveVec.y, 0.0f);
    if (testBox(nextPosY)) {
        if (moveVec.y < 0.0f) {
            isGrounded = true;
        }
        moveVec.y = 0.0f;
        velocity.y = 0.0f;
    } else {
        isGrounded = false;
    }

    // X Axis collision
    glm::vec3 nextPosX = position + glm::vec3(moveVec.x, 0.0f, 0.0f);
    if (testBox(nextPosX)) {
        moveVec.x = 0.0f;
        velocity.x = 0.0f;
    }

    // Z Axis collision
    glm::vec3 nextPosZ = position + glm::vec3(0.0f, 0.0f, moveVec.z);
    if (testBox(nextPosZ)) {
        moveVec.z = 0.0f;
        velocity.z = 0.0f;
    }
}
