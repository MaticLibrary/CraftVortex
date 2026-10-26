#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <cstdint>

// Chunk dimensions
constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Z = 16;
constexpr int CHUNK_HEIGHT = 128;
constexpr int CHUNK_VOLUME = CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_HEIGHT;

// World constants
constexpr int SEA_LEVEL = 40;
constexpr int WORLD_RENDER_DISTANCE = 8; // Chunks radius (17x17 chunks loaded = 289 chunks around player)

// Voxel Block IDs
enum BlockId : uint8_t {
    BLOCK_AIR = 0,
    BLOCK_GRASS = 1,
    BLOCK_DIRT = 2,
    BLOCK_STONE = 3,
    BLOCK_COBBLESTONE = 4,
    BLOCK_RUNE_STONE = 5,       // Ancient glowing rune block
    BLOCK_AMETHYST = 6,         // Glowing purple crystal
    BLOCK_EMERALD_MANA = 7,     // Glowing teal mana crystal
    BLOCK_GOLD_ORE = 8,         // Sparkling golden ore
    BLOCK_SANDSTONE = 9,        // Desert pyramid sandstone
    BLOCK_HIEROGLYPH = 10,      // Carved hieroglyphs for pyramids
    BLOCK_DARK_WOOD = 11,       // Spooky forest tree trunk
    BLOCK_SPOOKY_LEAVES = 12,   // Bioluminescent purple/indigo leaves
    BLOCK_WOOD_PLANKS = 13,     // Ruined city wood
    BLOCK_DUNGEON_BRICK = 14,   // Ancient temple brick
    BLOCK_GLOW_LANTERN = 15,    // Mystical glowing lantern
    BLOCK_WATER = 16,           // Ocean / lake water
    BLOCK_SAND = 17,            // Beach / desert sand
    BLOCK_OBSIDIAN = 18,        // Volcanic rock
    BLOCK_MOSS_GRASS = 19,
    BLOCK_SNOW = 20,
    BLOCK_MARSH_MUD = 21,
    BLOCK_BASALT = 22,
    BLOCK_MEADOW_GRASS = 23,
    BLOCK_BOOKSHELF = 24,
    BLOCK_CHEST = 25,
    BLOCK_IRON_CHAIN = 26,
    BLOCK_MARBLE = 27,
    BLOCK_GOLD_TRIM = 28,
    BLOCK_CRYSTAL_BRICK = 29,
    BLOCK_COUNT = 30
};

// Biomes for the huge island
enum BiomeType : uint8_t {
    BIOME_OCEAN = 0,
    BIOME_BEACH = 1,
    BIOME_SPOOKY_FOREST = 2,
    BIOME_ANCIENT_DESERT = 3,
    BIOME_CRYSTAL_PEAKS = 4,
    BIOME_RUINED_VALLEY = 5,
    BIOME_FROSTFALL = 6,
    BIOME_MISTY_MARSH = 7,
    BIOME_EMBERLANDS = 8,
    BIOME_MOONLIT_MEADOW = 9
};
