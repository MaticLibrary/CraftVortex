#pragma once
#include "Common.h"
#include "Camera.h"
#include "Raycast.h"
#include <array>

enum class ToolType : uint8_t { Pickaxe, Axe, Sword, Bow };
enum class CraftOutputType : uint8_t { Block, Tool, Arrows };

struct CraftRecipe {
    BlockId inputA;
    int amountA;
    BlockId inputB;
    int amountB;
    BlockId output;
    int outputAmount;
    CraftOutputType outputType = CraftOutputType::Block;
    ToolType toolOutput = ToolType::Pickaxe;
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    bool intersects(const AABB& other) const {
        return (min.x < other.max.x && max.x > other.min.x) &&
               (min.y < other.max.y && max.y > other.min.y) &&
               (min.z < other.max.z && max.z > other.min.z);
    }
};

class Player {
public:
    glm::vec3 position;
    Camera camera;
    glm::vec3 velocity{0.0f};

    bool isFlying = false;
    bool isGrounded = false;
    bool isSprinting = false;
    bool craftingOpen = false;
    bool inventoryOpen = false;
    bool chestOpen = false;
    glm::ivec3 openChestPosition{0};
    float health = 100.0f;
    std::array<int, BLOCK_COUNT> inventory{};
    std::array<bool, 4> ownedTools{true, false, false, false};
    int arrows = 24;
    ToolType selectedTool = ToolType::Pickaxe;

    // Hotbar setup
    static constexpr int HOTBAR_SLOTS = 15;
    static constexpr int CRAFT_RECIPE_COUNT = 13;
    BlockId hotbar[HOTBAR_SLOTS] = {
        BLOCK_GRASS,
        BLOCK_STONE,
        BLOCK_COBBLESTONE,
        BLOCK_RUNE_STONE,
        BLOCK_AMETHYST,
        BLOCK_SANDSTONE,
        BLOCK_WOOD_PLANKS,
        BLOCK_GLOW_LANTERN,
        BLOCK_BOOKSHELF,
        BLOCK_CHEST,
        BLOCK_IRON_CHAIN,
        BLOCK_MARBLE,
        BLOCK_GOLD_TRIM,
        BLOCK_CRYSTAL_BRICK,
        BLOCK_EMERALD_MANA
    };
    int selectedSlot = 0;

    Player(glm::vec3 spawnPos);

    void update(float dt, World& world, GLFWwindow* window);
    void handleMouseClick(int button, int action, World& world);
    void handleScroll(double yoffset);
    void takeDamage(float damage);
    void registerHit() { m_hitConfirmTimer = 0.18f; }
    float getDamageFlash() const { return m_damageFlash; }
    float getHitConfirm() const { return m_hitConfirmTimer; }
    const char* getToolName() const;
    float getAttackDamage() const;
    bool craftRecipe(int recipeIndex);
    bool canCraftRecipe(int recipeIndex) const;
    bool openChestAt(const glm::ivec3& position, const World& world);
    void closeMenus();
    void cycleTool();
    bool ownsTool(ToolType tool) const { return ownedTools[static_cast<int>(tool)]; }
    bool consumeArrow() { if (arrows <= 0) return false; --arrows; return true; }
    bool consumeBlockBreak(glm::ivec3& position, BlockId& id) {
        if (!m_hasBreakEvent) return false;
        position = m_lastBrokenBlock;
        id = m_lastBrokenBlockId;
        m_hasBreakEvent = false;
        return true;
    }
    float getMiningProgress() const { return m_miningProgress; }
    static CraftRecipe getCraftRecipe(int index);

    BlockId getSelectedBlock() const { return hotbar[selectedSlot]; }
    RaycastResult getLookTarget(const World& world) const;

    AABB getAABB() const {
        float halfW = 0.3f;
        return {
            glm::vec3(position.x - halfW, position.y, position.z - halfW),
            glm::vec3(position.x + halfW, position.y + 1.8f, position.z + halfW)
        };
    }

private:
    float m_width = 0.6f;
    float m_height = 1.8f;
    float m_eyeHeight = 1.62f;
    glm::vec3 m_spawnPosition{0.0f};
    bool m_miningActive = false;
    glm::ivec3 m_miningBlock{0};
    float m_miningProgress = 0.0f;
    float m_attackCooldown = 0.0f;
    float m_damageFlash = 0.0f;
    float m_hitConfirmTimer = 0.0f;
    bool m_qKeyLast = false;
    bool m_hasBreakEvent = false;
    glm::ivec3 m_lastBrokenBlock{0};
    BlockId m_lastBrokenBlockId = BLOCK_AIR;

    void updatePhysics(float dt, const World& world, const glm::vec3& inputDir);
    void resolveVoxelCollisions(const World& world, glm::vec3& moveVec);
    void updateMining(float dt, World& world);
};
