#pragma once
#include "Block.h"

class Chunk;

class Structures {
public:
    // Builds a majestic stepped ancient pyramid
    static void buildPyramid(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        int halfSize = 10;
        int height = 10;

        for (int step = 0; step < height; ++step) {
            int radius = halfSize - step;
            int y = worldY + step;

            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    bool isOuter = (dx == -radius || dx == radius || dz == -radius || dz == radius);
                    BlockId block = isOuter ? (step % 2 == 1 ? BLOCK_HIEROGLYPH : BLOCK_SANDSTONE) : BLOCK_SANDSTONE;
                    setWorldBlock(worldX + dx, y, worldZ + dz, block);
                }
            }
        }

        // Carve entrance passage from South side (+Z) into center
        for (int dz = 2; dz <= halfSize; ++dz) {
            for (int dy = 1; dy <= 3; ++dy) {
                setWorldBlock(worldX, worldY + dy, worldZ + dz, BLOCK_AIR);
            }
        }

        // Carve inner secret tomb chamber (5x5x4)
        for (int dx = -2; dx <= 2; ++dx) {
            for (int dy = 1; dy <= 4; ++dy) {
                for (int dz = -2; dz <= 2; ++dz) {
                    setWorldBlock(worldX + dx, worldY + dy, worldZ + dz, BLOCK_AIR);
                }
            }
        }

        // Inner altar with ancient treasures and glowing magic
        setWorldBlock(worldX, worldY + 1, worldZ, BLOCK_OBSIDIAN);
        setWorldBlock(worldX, worldY + 2, worldZ, BLOCK_RUNE_STONE);
        setWorldBlock(worldX - 1, worldY + 1, worldZ, BLOCK_GOLD_ORE);
        setWorldBlock(worldX + 1, worldY + 1, worldZ, BLOCK_GOLD_ORE);
        setWorldBlock(worldX, worldY + 1, worldZ - 1, BLOCK_AMETHYST);
        setWorldBlock(worldX, worldY + 1, worldZ + 1, BLOCK_EMERALD_MANA);

        // Four mystical lanterns in the corners of the tomb
        setWorldBlock(worldX - 2, worldY + 3, worldZ - 2, BLOCK_GLOW_LANTERN);
        setWorldBlock(worldX + 2, worldY + 3, worldZ - 2, BLOCK_GLOW_LANTERN);
        setWorldBlock(worldX - 2, worldY + 3, worldZ + 2, BLOCK_GLOW_LANTERN);
        setWorldBlock(worldX + 2, worldY + 3, worldZ + 2, BLOCK_GLOW_LANTERN);
    }

    // Builds a spooky giant fantasy tree
    static void buildSpookyTree(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        int trunkHeight = 12 + (abs(worldX * 7 + worldZ * 13) % 6);

        // Roots
        for (int dy = 0; dy <= 2; ++dy) {
            setWorldBlock(worldX - 1, worldY + dy, worldZ, BLOCK_DARK_WOOD);
            setWorldBlock(worldX + 1, worldY + dy, worldZ, BLOCK_DARK_WOOD);
            setWorldBlock(worldX, worldY + dy, worldZ - 1, BLOCK_DARK_WOOD);
            setWorldBlock(worldX, worldY + dy, worldZ + 1, BLOCK_DARK_WOOD);
        }

        // Main trunk
        for (int y = 0; y < trunkHeight; ++y) {
            setWorldBlock(worldX, worldY + y, worldZ, BLOCK_DARK_WOOD);
        }

        // Twisted canopy
        int canopyY = worldY + trunkHeight - 2;
        int leafRadius = 4;

        for (int dy = -2; dy <= 3; ++dy) {
            int rad = leafRadius - abs(dy);
            for (int dx = -rad; dx <= rad; ++dx) {
                for (int dz = -rad; dz <= rad; ++dz) {
                    if (dx * dx + dz * dz <= rad * rad + 1) {
                        int px = worldX + dx;
                        int py = canopyY + dy;
                        int pz = worldZ + dz;
                        // Avoid overwriting trunk
                        if (dx == 0 && dz == 0 && dy < 2) continue;
                        setWorldBlock(px, py, pz, BLOCK_SPOOKY_LEAVES);
                    }
                }
            }
        }

        // Lantern hanging under foliage
        setWorldBlock(worldX + 2, canopyY - 2, worldZ, BLOCK_GLOW_LANTERN);
        setWorldBlock(worldX - 2, canopyY - 2, worldZ, BLOCK_GLOW_LANTERN);
    }

    // Builds an ancient ruined fortress watchtower
    static void buildRuinedTower(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        int towerHeight = 14;
        int radius = 3;

        for (int y = 0; y < towerHeight; ++y) {
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    bool isWall = (dx == -radius || dx == radius || dz == -radius || dz == radius);
                    if (isWall) {
                        // Weathered ruined gaps in high walls
                        if (y > 9 && ((dx + dz + y) % 3 == 0)) {
                            continue; // Broken opening
                        }
                        BlockId wallMat = ((dx * 3 + y * 5 + dz) % 4 == 0) ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE;
                        setWorldBlock(worldX + dx, worldY + y, worldZ + dz, wallMat);
                    } else if (y == 0 || y == 6 || y == 12) {
                        // Wooden floors
                        setWorldBlock(worldX + dx, worldY + y, worldZ + dz, BLOCK_WOOD_PLANKS);
                    }
                }
            }
        }

        // Entrance arch
        for (int dy = 1; dy <= 3; ++dy) {
            setWorldBlock(worldX, worldY + dy, worldZ + radius, BLOCK_AIR);
        }

        // Magic rune stone on top floor
        setWorldBlock(worldX, worldY + 13, worldZ, BLOCK_RUNE_STONE);
        setWorldBlock(worldX, worldY + 14, worldZ, BLOCK_GLOW_LANTERN);
    }

    // Builds a ruined village hut
    static void buildRuinedHut(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        int width = 5;
        int depth = 6;
        int height = 4;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                for (int z = 0; z < depth; ++z) {
                    bool wall = (x == 0 || x == width - 1 || z == 0 || z == depth - 1);
                    if (y == 0) {
                        setWorldBlock(worldX + x, worldY, worldZ + z, BLOCK_COBBLESTONE);
                    } else if (wall) {
                        // Door
                        if (z == depth - 1 && x == 2 && y <= 2) {
                            setWorldBlock(worldX + x, worldY + y, worldZ + z, BLOCK_AIR);
                        } else {
                            BlockId mat = (y == 1) ? BLOCK_COBBLESTONE : BLOCK_WOOD_PLANKS;
                            setWorldBlock(worldX + x, worldY + y, worldZ + z, mat);
                        }
                    }
                }
            }
        }

        // Dilapidated roof
        for (int x = 0; x < width; ++x) {
            for (int z = 0; z < depth; ++z) {
                if ((x + z) % 3 != 0) { // Holes in ruined roof
                    setWorldBlock(worldX + x, worldY + height, worldZ + z, BLOCK_WOOD_PLANKS);
                }
            }
        }
        // Small chest/crystal inside
        setWorldBlock(worldX + 1, worldY + 1, worldZ + 1, BLOCK_AMETHYST);
    }

    // A compact stone keep with a walkable courtyard and four corner towers.
    static void buildCastle(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        constexpr int half = 12;
        constexpr int wallHeight = 7;
        for (int y = 0; y < wallHeight; ++y) {
            for (int d = -half; d <= half; ++d) {
                for (int side : {-half, half}) {
                    // Leave a three-block gate in the front wall.
                    if (!(side == half && d >= -1 && d <= 1 && y <= 3))
                        setWorldBlock(worldX + d, worldY + y, worldZ + side,
                                      y == wallHeight - 1 ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE);
                    setWorldBlock(worldX + side, worldY + y, worldZ + d,
                                  y == wallHeight - 1 ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE);
                }
            }
        }

        const int cornerX[] = {-half, half};
        const int cornerZ[] = {-half, half};
        for (int cx : cornerX) {
            for (int cz : cornerZ) {
                for (int y = 0; y < 15; ++y) {
                    for (int dx = -2; dx <= 2; ++dx) {
                        for (int dz = -2; dz <= 2; ++dz) {
                            if (dx == -2 || dx == 2 || dz == -2 || dz == 2 || y == 0 || y == 7 || y == 14)
                                setWorldBlock(worldX + cx + dx, worldY + y, worldZ + cz + dz,
                                              y == 14 ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE);
                        }
                    }
                }
                for (int y = 1; y <= 3; ++y)
                    setWorldBlock(worldX + cx, worldY + y, worldZ + cz, BLOCK_AIR);
                setWorldBlock(worldX + cx, worldY + 15, worldZ + cz, BLOCK_GLOW_LANTERN);
            }
        }

        // Small keep in the middle of the courtyard.
        for (int y = 0; y < 8; ++y) {
            for (int dx = -3; dx <= 3; ++dx) {
                for (int dz = -3; dz <= 3; ++dz) {
                    bool wall = dx == -3 || dx == 3 || dz == -3 || dz == 3;
                    if (wall || y == 0 || y == 7)
                        setWorldBlock(worldX + dx, worldY + y, worldZ + dz,
                                      y == 7 ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE);
                }
            }
        }
        for (int y = 1; y <= 3; ++y) setWorldBlock(worldX, worldY + y, worldZ + 3, BLOCK_AIR);
        setWorldBlock(worldX, worldY + 8, worldZ, BLOCK_RUNE_STONE);
    }

    // A large, layered gothic fortress silhouette: curtain wall, gatehouse,
    // corner towers, an inner keep and a tall rune-lit central spire.
    static void buildGothicCastle(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        constexpr int half = 27;
        constexpr int wallTop = 13;

        // Broad stone courtyard gives the keep a readable, level footprint.
        for (int dx = -half; dx <= half; ++dx) {
            for (int dz = -half; dz <= half; ++dz) {
                setWorldBlock(worldX + dx, worldY, worldZ + dz,
                              ((dx + dz) % 9 == 0) ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE);
            }
        }

        // Curtain walls with recessed bands, arrow slits and crenellations.
        for (int y = 1; y <= wallTop; ++y) {
            for (int d = -half; d <= half; ++d) {
                bool gateOpening = d >= -5 && d <= 5 && y <= 8;
                if (!gateOpening) {
                    bool slit = (y == 7 || y == 8) && d % 8 == 0 && std::abs(d) > 7;
                    if (!slit) {
                        BlockId mat = (y == 4 || y == 9) ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE;
                        setWorldBlock(worldX + d, worldY + y, worldZ + half, mat);
                        setWorldBlock(worldX + d, worldY + y, worldZ - half, mat);
                        setWorldBlock(worldX + half, worldY + y, worldZ + d, mat);
                        setWorldBlock(worldX - half, worldY + y, worldZ + d, mat);
                    }
                }
            }
        }
        for (int d = -half; d <= half; ++d) {
            if ((d + half) % 2 == 0) {
                for (int side : {-half, half}) {
                    setWorldBlock(worldX + d, worldY + wallTop + 1, worldZ + side, BLOCK_DUNGEON_BRICK);
                    setWorldBlock(worldX + side, worldY + wallTop + 1, worldZ + d, BLOCK_DUNGEON_BRICK);
                }
            }
        }

        // The gate is a deep passage with a raised arch and ironwood portcullis.
        for (int y = 1; y <= 9; ++y) {
            for (int dx = -6; dx <= 6; ++dx) {
                if (std::abs(dx) >= 5 || y >= 8)
                    setWorldBlock(worldX + dx, worldY + y, worldZ + half - 1, BLOCK_DUNGEON_BRICK);
            }
        }
        for (int dx = -4; dx <= 4; ++dx)
            setWorldBlock(worldX + dx, worldY + 1, worldZ + half, BLOCK_DUNGEON_BRICK);
        for (int dx = -4; dx <= 4; dx += 2) {
            for (int y = 2; y <= 7; ++y)
                setWorldBlock(worldX + dx, worldY + y, worldZ + half - 2, BLOCK_DARK_WOOD);
        }
        for (int dx : {-7, 7}) {
            setWorldBlock(worldX + dx, worldY + 10, worldZ + half - 1, BLOCK_GLOW_LANTERN);
            setWorldBlock(worldX + dx, worldY + 11, worldZ + half - 1, BLOCK_GLOW_LANTERN);
        }

        // Square tower shafts have floors, corner ribs, narrow window cuts,
        // and stepped roofs that rise above the curtain wall.
        auto buildTower = [&](int tx, int tz, int height, int radius) {
            for (int y = 1; y <= height; ++y) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    for (int dz = -radius; dz <= radius; ++dz) {
                        bool edge = std::abs(dx) == radius || std::abs(dz) == radius;
                        bool rib = (std::abs(dx) == radius && std::abs(dz) == radius);
                        bool floor = y == 1 || y == 8 || y == 15 || y == 22 || y == height - 2;
                        bool windowGap = edge && y > 4 && y < height - 3 && y % 8 >= 3 && y % 8 <= 4 &&
                                         ((dx == 0 && std::abs(dz) == radius) || (dz == 0 && std::abs(dx) == radius));
                        if (windowGap) continue;
                        if (edge || rib || floor)
                            setWorldBlock(tx + dx, worldY + y, tz + dz,
                                          (y % 8 == 0 || rib) ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE);
                    }
                }
            }
            int parapetY = worldY + height;
            for (int d = -radius; d <= radius; ++d) {
                if (d % 2 == 0) {
                    setWorldBlock(tx + d, parapetY, tz - radius, BLOCK_DUNGEON_BRICK);
                    setWorldBlock(tx + d, parapetY, tz + radius, BLOCK_DUNGEON_BRICK);
                    setWorldBlock(tx - radius, parapetY, tz + d, BLOCK_DUNGEON_BRICK);
                    setWorldBlock(tx + radius, parapetY, tz + d, BLOCK_DUNGEON_BRICK);
                }
            }
            int roofBase = height + 1;
            for (int layer = 0; layer < 6; ++layer) {
                int r = radius - layer;
                if (r < 0) break;
                for (int dx = -r; dx <= r; ++dx) {
                    for (int dz = -r; dz <= r; ++dz) {
                        bool rim = std::abs(dx) == r || std::abs(dz) == r;
                        if (rim || layer == 5)
                            setWorldBlock(tx + dx, worldY + roofBase + layer, tz + dz,
                                          layer % 2 == 0 ? BLOCK_DUNGEON_BRICK : BLOCK_OBSIDIAN);
                    }
                }
            }
            setWorldBlock(tx, worldY + roofBase + 6, tz, BLOCK_RUNE_STONE);
            setWorldBlock(tx, worldY + roofBase + 7, tz, BLOCK_GLOW_LANTERN);
        };

        buildTower(worldX - half + 1, worldZ - half + 1, 27, 5);
        buildTower(worldX + half - 1, worldZ - half + 1, 31, 5);
        buildTower(worldX - half + 1, worldZ + half - 1, 34, 5);
        buildTower(worldX + half - 1, worldZ + half - 1, 29, 5);
        buildTower(worldX - 7, worldZ + half - 1, 24, 4);
        buildTower(worldX + 7, worldZ + half - 1, 24, 4);

        // Main keep and the narrow central donjon.
        for (int y = 1; y <= 25; ++y) {
            for (int dx = -13; dx <= 13; ++dx) {
                for (int dz = -13; dz <= 13; ++dz) {
                    bool edge = std::abs(dx) == 13 || std::abs(dz) == 13;
                    bool floor = y == 1 || y == 9 || y == 17 || y == 24;
                    bool doorway = dz == 13 && std::abs(dx) <= 2 && y <= 5;
                    bool window = edge && std::abs(dx) <= 1 && y % 8 >= 3 && y % 8 <= 5 && y < 23;
                    if ((edge || floor) && !doorway && !window)
                        setWorldBlock(worldX + dx, worldY + y, worldZ + dz,
                                      y % 8 == 0 ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE);
                }
            }
        }
        for (int d = -13; d <= 13; d += 2) {
            setWorldBlock(worldX + d, worldY + 26, worldZ - 13, BLOCK_DUNGEON_BRICK);
            setWorldBlock(worldX + d, worldY + 26, worldZ + 13, BLOCK_DUNGEON_BRICK);
            setWorldBlock(worldX - 13, worldY + 26, worldZ + d, BLOCK_DUNGEON_BRICK);
            setWorldBlock(worldX + 13, worldY + 26, worldZ + d, BLOCK_DUNGEON_BRICK);
        }
        for (int y = 1; y <= 42; ++y) {
            for (int dx = -5; dx <= 5; ++dx) {
                for (int dz = -5; dz <= 5; ++dz) {
                    bool edge = std::abs(dx) == 5 || std::abs(dz) == 5;
                    bool window = edge && y % 10 >= 4 && y % 10 <= 6 &&
                                  ((dx == 0 && std::abs(dz) == 5) || (dz == 0 && std::abs(dx) == 5));
                    if (edge && !window)
                        setWorldBlock(worldX + dx, worldY + y, worldZ + dz,
                                      y % 9 == 0 ? BLOCK_DUNGEON_BRICK : BLOCK_COBBLESTONE);
                }
            }
            if (y % 7 == 0)
                setWorldBlock(worldX, worldY + y, worldZ, BLOCK_WOOD_PLANKS);
        }
        for (int layer = 0; layer < 7; ++layer) {
            int radius = 5 - layer;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (std::abs(dx) == radius || std::abs(dz) == radius)
                        setWorldBlock(worldX + dx, worldY + 43 + layer, worldZ + dz,
                                      layer % 2 == 0 ? BLOCK_OBSIDIAN : BLOCK_DUNGEON_BRICK);
                }
            }
        }
        setWorldBlock(worldX, worldY + 50, worldZ, BLOCK_RUNE_STONE);
        setWorldBlock(worldX, worldY + 51, worldZ, BLOCK_GLOW_LANTERN);

        // Raised approach, torch posts and a courtyard altar.
        for (int dz = half + 1; dz <= half + 13; ++dz) {
            for (int dx = -4; dx <= 4; ++dx) {
                setWorldBlock(worldX + dx, worldY + 1, worldZ + dz,
                              std::abs(dx) == 4 ? BLOCK_COBBLESTONE : BLOCK_DUNGEON_BRICK);
            }
        }
        for (int side : {-1, 1}) {
            for (int dz = half + 2; dz <= half + 12; dz += 5) {
                setWorldBlock(worldX + side * 6, worldY + 2, worldZ + dz, BLOCK_COBBLESTONE);
                setWorldBlock(worldX + side * 6, worldY + 3, worldZ + dz, BLOCK_GLOW_LANTERN);
            }
        }
        setWorldBlock(worldX, worldY + 2, worldZ - 4, BLOCK_OBSIDIAN);
        setWorldBlock(worldX, worldY + 3, worldZ - 4, BLOCK_RUNE_STONE);

        // The keep is furnished like a lived-in royal stronghold. A dark
        // crystal runner leads from the gate to a gilded throne dais.
        for (int dz = 11; dz >= -8; --dz) {
            for (int dx = -2; dx <= 2; ++dx) {
                BlockId floor = std::abs(dx) == 2 ? BLOCK_GOLD_TRIM :
                                (dx == 0 ? BLOCK_CRYSTAL_BRICK : BLOCK_MARBLE);
                setWorldBlock(worldX + dx, worldY + 1, worldZ + dz, floor);
            }
        }

        // Throne, raised dais and luminous wall crest.
        for (int dx = -2; dx <= 2; ++dx)
            setWorldBlock(worldX + dx, worldY + 2, worldZ - 9,
                          std::abs(dx) == 2 ? BLOCK_GOLD_TRIM : BLOCK_MARBLE);
        setWorldBlock(worldX, worldY + 3, worldZ - 9, BLOCK_DARK_WOOD);
        setWorldBlock(worldX, worldY + 4, worldZ - 9, BLOCK_GOLD_TRIM);
        setWorldBlock(worldX, worldY + 5, worldZ - 9, BLOCK_RUNE_STONE);
        for (int side : {-1, 1}) {
            for (int y = 2; y <= 7; ++y)
                setWorldBlock(worldX + side * 4, worldY + y, worldZ - 10, BLOCK_GOLD_TRIM);
            setWorldBlock(worldX + side * 2, worldY + 5, worldZ - 12, BLOCK_CRYSTAL_BRICK);
        }

        // Two-storey library galleries and a winding, jumpable block stair.
        for (int z = -7; z <= 6; ++z) {
            if (z % 3 != 0) {
                setWorldBlock(worldX - 11, worldY + 2, worldZ + z, BLOCK_BOOKSHELF);
                setWorldBlock(worldX - 11, worldY + 3, worldZ + z, BLOCK_BOOKSHELF);
                setWorldBlock(worldX - 11, worldY + 4, worldZ + z, BLOCK_BOOKSHELF);
            }
        }
        for (int floorBase : {1, 9, 17}) {
            for (int step = 1; step <= 8; ++step)
                setWorldBlock(worldX + 9, worldY + floorBase + step, worldZ + 11 - step, BLOCK_MARBLE);
        }
        for (int z = -7; z <= 3; ++z) {
            setWorldBlock(worldX + 11, worldY + 10, worldZ + z, BLOCK_BOOKSHELF);
            setWorldBlock(worldX + 11, worldY + 11, worldZ + z, BLOCK_BOOKSHELF);
            if (z % 2 == 0) setWorldBlock(worldX + 11, worldY + 12, worldZ + z, BLOCK_BOOKSHELF);
        }
        for (int z = -8; z <= -4; z += 2)
            setWorldBlock(worldX - 10, worldY + 18, worldZ + z, BLOCK_BOOKSHELF);

        // Treasure chests are placed in alcoves beside the hall and on the
        // upper gallery; chains support a three-part crystal chandelier.
        for (int side : {-1, 1}) {
            setWorldBlock(worldX + side * 8, worldY + 2, worldZ + 5, BLOCK_CHEST);
            setWorldBlock(worldX + side * 8, worldY + 2, worldZ + 7, BLOCK_CHEST);
            setWorldBlock(worldX + side * 8, worldY + 10, worldZ - 5, BLOCK_CHEST);
        }
        for (int x : {-3, 0, 3}) {
            for (int y = 6; y <= 8; ++y)
                setWorldBlock(worldX + x, worldY + y, worldZ + 1, BLOCK_IRON_CHAIN);
        }
        setWorldBlock(worldX - 3, worldY + 5, worldZ + 1, BLOCK_GLOW_LANTERN);
        setWorldBlock(worldX, worldY + 5, worldZ + 1, BLOCK_CRYSTAL_BRICK);
        setWorldBlock(worldX + 3, worldY + 5, worldZ + 1, BLOCK_GLOW_LANTERN);

        // Gilded corner piers and small crystal reliquaries give the inner
        // courtyard and the donjon a distinct fantasy palette.
        for (int sideX : {-1, 1}) {
            for (int sideZ : {-1, 1}) {
                for (int y = 2; y <= 6; ++y)
                    setWorldBlock(worldX + sideX * 10, worldY + y, worldZ + sideZ * 10, BLOCK_GOLD_TRIM);
                setWorldBlock(worldX + sideX * 10, worldY + 7, worldZ + sideZ * 10, BLOCK_AMETHYST);
            }
        }
    }

    // A small settlement: three timber houses around a lantern-lit well.
    static void buildVillage(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        buildRuinedHut(worldX - 10, worldY, worldZ - 3, setWorldBlock);
        buildRuinedHut(worldX + 5, worldY, worldZ - 3, setWorldBlock);
        buildRuinedHut(worldX - 2, worldY, worldZ + 7, setWorldBlock);

        for (int dx = -2; dx <= 2; ++dx) {
            for (int dz = -2; dz <= 2; ++dz) {
                bool rim = std::abs(dx) == 2 || std::abs(dz) == 2;
                setWorldBlock(worldX + dx, worldY, worldZ + dz,
                              rim ? BLOCK_COBBLESTONE : BLOCK_WATER);
            }
        }
        setWorldBlock(worldX, worldY + 1, worldZ, BLOCK_GLOW_LANTERN);

        // Footpaths connect the homes to the well.
        for (int d = -10; d <= 10; ++d) {
            setWorldBlock(worldX + d, worldY, worldZ + 1, BLOCK_SANDSTONE);
            if (d % 2 == 0) setWorldBlock(worldX, worldY, worldZ + d, BLOCK_SANDSTONE);
        }
    }

    // A columned shrine with a stepped roof and a bright altar.
    static void buildTemple(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        constexpr int halfX = 7;
        constexpr int halfZ = 9;
        for (int dx = -halfX; dx <= halfX; ++dx) {
            for (int dz = -halfZ; dz <= halfZ; ++dz) {
                setWorldBlock(worldX + dx, worldY, worldZ + dz, BLOCK_SANDSTONE);
                bool column = (std::abs(dx) == halfX && std::abs(dz) == halfZ) ||
                              (std::abs(dx) == halfX && dz % 4 == 0);
                if (column) {
                    for (int y = 1; y <= 7; ++y)
                        setWorldBlock(worldX + dx, worldY + y, worldZ + dz, BLOCK_HIEROGLYPH);
                }
            }
        }
        for (int layer = 0; layer < 3; ++layer) {
            for (int dx = -halfX + layer; dx <= halfX - layer; ++dx) {
                for (int dz = -halfZ + layer; dz <= halfZ - layer; ++dz) {
                    setWorldBlock(worldX + dx, worldY + 8 + layer, worldZ + dz,
                                  layer == 1 ? BLOCK_HIEROGLYPH : BLOCK_SANDSTONE);
                }
            }
        }
        // Open front entrance and line the central aisle with lanterns.
        for (int y = 1; y <= 4; ++y) {
            for (int dx = -1; dx <= 1; ++dx)
                setWorldBlock(worldX + dx, worldY + y, worldZ + halfZ, BLOCK_AIR);
        }
        setWorldBlock(worldX, worldY + 1, worldZ, BLOCK_OBSIDIAN);
        setWorldBlock(worldX, worldY + 2, worldZ, BLOCK_RUNE_STONE);
        setWorldBlock(worldX - 4, worldY + 2, worldZ - 5, BLOCK_GLOW_LANTERN);
        setWorldBlock(worldX + 4, worldY + 2, worldZ - 5, BLOCK_GLOW_LANTERN);
    }

    static void buildFrozenPine(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        int height = 9 + (std::abs(worldX * 13 + worldZ * 7) % 5);
        for (int y = 0; y < height; ++y)
            setWorldBlock(worldX, worldY + y, worldZ, BLOCK_DARK_WOOD);
        for (int layer = 0; layer < 6; ++layer) {
            int radius = 4 - layer / 2;
            int y = worldY + height - 6 + layer;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    if (std::abs(dx) + std::abs(dz) <= radius + 1)
                        setWorldBlock(worldX + dx, y, worldZ + dz, BLOCK_SNOW);
                }
            }
        }
    }

    static void buildBogTree(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        int height = 6 + (std::abs(worldX * 5 + worldZ * 11) % 4);
        for (int y = 0; y < height; ++y) {
            setWorldBlock(worldX, worldY + y, worldZ, BLOCK_DARK_WOOD);
            if (y < 3 && y % 2 == 0) {
                setWorldBlock(worldX - 1, worldY + y, worldZ, BLOCK_DARK_WOOD);
                setWorldBlock(worldX + 1, worldY + y, worldZ, BLOCK_DARK_WOOD);
            }
        }
        int canopyY = worldY + height - 1;
        for (int dx = -3; dx <= 3; ++dx) {
            for (int dz = -3; dz <= 3; ++dz) {
                if (dx * dx + dz * dz <= 10)
                    setWorldBlock(worldX + dx, canopyY, worldZ + dz, BLOCK_SPOOKY_LEAVES);
            }
        }
        setWorldBlock(worldX, canopyY - 1, worldZ, BLOCK_DARK_WOOD);
        setWorldBlock(worldX + 2, canopyY - 1, worldZ, BLOCK_GLOW_LANTERN);
    }

    static void buildVolcanicVent(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        for (int layer = 0; layer < 5; ++layer) {
            int radius = 5 - layer;
            int y = worldY + layer;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    int edge = std::max(std::abs(dx), std::abs(dz));
                    if (edge >= radius - 1)
                        setWorldBlock(worldX + dx, y, worldZ + dz,
                                      ((dx + dz + layer) % 4 == 0) ? BLOCK_OBSIDIAN : BLOCK_BASALT);
                }
            }
        }
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dz = -1; dz <= 1; ++dz)
                setWorldBlock(worldX + dx, worldY + 4, worldZ + dz, BLOCK_GLOW_LANTERN);
        }
    }

    static void buildMeadowRuin(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        for (int dx = -4; dx <= 4; ++dx) {
            for (int dz = -4; dz <= 4; ++dz) {
                int dist2 = dx * dx + dz * dz;
                if (dist2 >= 10 && dist2 <= 20 && (dx + dz) % 3 != 0)
                    setWorldBlock(worldX + dx, worldY, worldZ + dz, BLOCK_COBBLESTONE);
            }
        }
        setWorldBlock(worldX, worldY, worldZ, BLOCK_MOSS_GRASS);
        setWorldBlock(worldX, worldY + 1, worldZ, BLOCK_EMERALD_MANA);
        setWorldBlock(worldX - 1, worldY, worldZ, BLOCK_MEADOW_GRASS);
        setWorldBlock(worldX + 1, worldY, worldZ, BLOCK_MEADOW_GRASS);
        setWorldBlock(worldX, worldY, worldZ - 1, BLOCK_MEADOW_GRASS);
        setWorldBlock(worldX, worldY, worldZ + 1, BLOCK_MEADOW_GRASS);
    }

    // Builds a glowing crystal spire
    static void buildCrystalSpire(int worldX, int worldY, int worldZ, auto setWorldBlock) {
        int height = 5 + (abs(worldX * 11 + worldZ * 17) % 5);
        BlockId crystalType = (worldX % 2 == 0) ? BLOCK_AMETHYST : BLOCK_EMERALD_MANA;

        for (int y = 0; y < height; ++y) {
            setWorldBlock(worldX, worldY + y, worldZ, crystalType);
            if (y < height - 2) {
                if (y % 2 == 0) setWorldBlock(worldX + 1, worldY + y, worldZ, crystalType);
                if (y % 3 == 0) setWorldBlock(worldX - 1, worldY + y, worldZ, crystalType);
                if (y % 2 == 1) setWorldBlock(worldX, worldY + y, worldZ + 1, crystalType);
            }
        }
    }
};
