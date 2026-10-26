#pragma once
#include "../engine/Common.h"

struct BlockFaceTex {
    int top;
    int bottom;
    int front; // North (+Z)
    int back;  // South (-Z)
    int left;  // West (-X)
    int right; // East (+X)
};

struct BlockData {
    std::string name;
    bool isSolid;
    bool isTransparent;
    bool isEmissive;
    float emitR, emitG, emitB;
    BlockFaceTex textures;
};

class BlockRegistry {
public:
    static const BlockData& get(BlockId id) {
        init();
        if (id >= BLOCK_COUNT) return registry[BLOCK_AIR];
        return registry[id];
    }

private:
    static inline bool initialized = false;
    static inline BlockData registry[BLOCK_COUNT];

    static void init() {
        if (initialized) return;
        initialized = true;

        // Tile IDs on 8x8 texture atlas (0 to 63):
        // 0: Air
        // 1: Grass Top, 2: Grass Side, 3: Dirt
        // 4: Stone, 5: Cobblestone
        // 6: Rune Stone (Cyan glyphs)
        // 7: Amethyst Crystal (Purple glow)
        // 8: Emerald Mana (Teal glow)
        // 9: Gold Ore
        // 10: Sandstone, 11: Hieroglyphic Sandstone
        // 12: Dark Wood Trunk, 13: Dark Wood Top
        // 14: Spooky Leaves (Indigo/Violet)
        // 15: Wood Planks
        // 16: Dungeon Brick
        // 17: Glow Lantern (Warm Gold/Amber)
        // 18: Water
        // 19: Sand
        // 20: Obsidian

        registry[BLOCK_AIR] = { "Powietrze", false, true, false, 0, 0, 0, {0,0,0,0,0,0} };
        
        registry[BLOCK_GRASS] = { "Mistyczna Trawa", true, false, false, 0, 0, 0, {1, 3, 2, 2, 2, 2} };
        registry[BLOCK_DIRT] = { "Ziemia", true, false, false, 0, 0, 0, {3, 3, 3, 3, 3, 3} };
        registry[BLOCK_STONE] = { "Kamień Pradawny", true, false, false, 0, 0, 0, {4, 4, 4, 4, 4, 4} };
        registry[BLOCK_COBBLESTONE] = { "Bruk Forteczny", true, false, false, 0, 0, 0, {5, 5, 5, 5, 5, 5} };

        registry[BLOCK_RUNE_STONE] = { "Runiczny Kamień", true, false, true, 0.2f, 0.9f, 1.0f, {6, 6, 6, 6, 6, 6} };
        registry[BLOCK_AMETHYST] = { "Kryształ Ametystu", true, false, true, 0.9f, 0.2f, 1.0f, {7, 7, 7, 7, 7, 7} };
        registry[BLOCK_EMERALD_MANA] = { "Kryształ Many", true, false, true, 0.1f, 1.0f, 0.8f, {8, 8, 8, 8, 8, 8} };
        registry[BLOCK_GOLD_ORE] = { "Ruda Złota", true, false, false, 0, 0, 0, {9, 9, 9, 9, 9, 9} };

        registry[BLOCK_SANDSTONE] = { "Piaskowiec Piramid", true, false, false, 0, 0, 0, {10, 10, 10, 10, 10, 10} };
        registry[BLOCK_HIEROGLYPH] = { "Hieroglify Faraona", true, false, false, 0, 0, 0, {10, 10, 11, 11, 11, 11} };

        registry[BLOCK_DARK_WOOD] = { "Mroczny Pień", true, false, false, 0, 0, 0, {13, 13, 12, 12, 12, 12} };
        registry[BLOCK_SPOOKY_LEAVES] = { "Bioluminescencyjne Liście", true, true, true, 0.4f, 0.1f, 0.7f, {14, 14, 14, 14, 14, 14} };

        registry[BLOCK_WOOD_PLANKS] = { "Prastare Deski", true, false, false, 0, 0, 0, {15, 15, 15, 15, 15, 15} };
        registry[BLOCK_DUNGEON_BRICK] = { "Cegła Loży Śmierci", true, false, false, 0, 0, 0, {16, 16, 16, 16, 16, 16} };
        registry[BLOCK_GLOW_LANTERN] = { "Mistyczna Latarnia", true, false, true, 1.0f, 0.85f, 0.3f, {17, 17, 17, 17, 17, 17} };

        registry[BLOCK_WATER] = { "Mistyczna Woda", false, true, false, 0, 0, 0, {18, 18, 18, 18, 18, 18} };
        registry[BLOCK_SAND] = { "Złocisty Piasek", true, false, false, 0, 0, 0, {19, 19, 19, 19, 19, 19} };
        registry[BLOCK_OBSIDIAN] = { "Obsydian Piekielny", true, false, false, 0, 0, 0, {20, 20, 20, 20, 20, 20} };
        registry[BLOCK_MOSS_GRASS] = { "Mechowata Trawa", true, false, false, 0, 0, 0, {21, 3, 21, 21, 21, 21} };
        registry[BLOCK_SNOW] = { "Wieczny Śnieg", true, false, false, 0, 0, 0, {22, 22, 22, 22, 22, 22} };
        registry[BLOCK_MARSH_MUD] = { "Bagienne Torfowisko", true, false, false, 0, 0, 0, {23, 23, 23, 23, 23, 23} };
        registry[BLOCK_BASALT] = { "Wulkaniczny Bazalt", true, false, false, 0, 0, 0, {24, 24, 24, 24, 24, 24} };
        registry[BLOCK_MEADOW_GRASS] = { "Srebrzysta Łąka", true, false, false, 0, 0, 0, {25, 3, 25, 25, 25, 25} };
        registry[BLOCK_BOOKSHELF] = { "Bookshelf", true, false, false, 0, 0, 0, {26, 26, 26, 26, 26, 26} };
        registry[BLOCK_CHEST] = { "Treasure Chest", true, false, false, 0, 0, 0, {27, 27, 27, 27, 27, 27} };
        registry[BLOCK_IRON_CHAIN] = { "Iron Chain", false, true, false, 0, 0, 0, {28, 28, 28, 28, 28, 28} };
        registry[BLOCK_MARBLE] = { "Pale Marble", true, false, false, 0, 0, 0, {29, 29, 29, 29, 29, 29} };
        registry[BLOCK_GOLD_TRIM] = { "Gilded Stone", true, false, false, 0, 0, 0, {30, 30, 30, 30, 30, 30} };
        registry[BLOCK_CRYSTAL_BRICK] = { "Crystal Brick", true, false, true, 0.32f, 0.12f, 0.72f, {31, 31, 31, 31, 31, 31} };
    }
};
