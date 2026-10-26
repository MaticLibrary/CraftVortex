#include "TextureAtlas.h"

TextureAtlas::TextureAtlas() {
    m_pixels.resize(ATLAS_SIZE * ATLAS_SIZE * 4, 0);
}

TextureAtlas::~TextureAtlas() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

void TextureAtlas::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x < 0 || x >= ATLAS_SIZE || y < 0 || y >= ATLAS_SIZE) return;
    int idx = (y * ATLAS_SIZE + x) * 4;
    m_pixels[idx + 0] = r;
    m_pixels[idx + 1] = g;
    m_pixels[idx + 2] = b;
    m_pixels[idx + 3] = a;
}

void TextureAtlas::drawTilePixel(int tileX, int tileY, int px, int py, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    int x = tileX * TILE_SIZE + px;
    int y = tileY * TILE_SIZE + py;
    setPixel(x, y, r, g, b, a);
}

void TextureAtlas::getTileUVs(int tileIndex, float& u0, float& v0, float& u1, float& v1) const {
    int tileX = tileIndex % TILES_PER_ROW;
    int tileY = tileIndex / TILES_PER_ROW;

    float eps = 0.001f; // Avoid texture bleeding
    u0 = (float)tileX / (float)TILES_PER_ROW + eps;
    v0 = (float)tileY / (float)TILES_PER_ROW + eps;
    u1 = (float)(tileX + 1) / (float)TILES_PER_ROW - eps;
    v1 = (float)(tileY + 1) / (float)TILES_PER_ROW - eps;
}

void TextureAtlas::paintGrassTop(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int noise = ((x * 7 + y * 13) % 23) - 10;
            uint8_t g = (uint8_t)std::clamp(145 + noise * 4, 80, 220);
            uint8_t r = (uint8_t)std::clamp(55 + noise * 2, 20, 110);
            uint8_t b = (uint8_t)std::clamp(45 + noise * 2, 10, 90);
            // Occasional magical emerald flower or dew
            if ((x * 19 + y * 31) % 47 == 0) {
                r = 110; g = 255; b = 210;
            }
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintGrassSide(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int drip = ((x * 11) % 4) + 3; // grass hangs down 3-6 pixels
            if (y < drip) {
                int noise = ((x * 7 + y * 11) % 17) - 8;
                uint8_t g = (uint8_t)std::clamp(135 + noise * 3, 70, 200);
                uint8_t r = (uint8_t)std::clamp(50 + noise * 2, 20, 95);
                uint8_t b = (uint8_t)std::clamp(40 + noise * 2, 10, 80);
                drawTilePixel(tx, ty, x, y, r, g, b);
            } else {
                int noise = ((x * 13 + y * 19) % 29) - 14;
                uint8_t r = (uint8_t)std::clamp(96 + noise * 2, 60, 140);
                uint8_t g = (uint8_t)std::clamp(64 + noise * 2, 40, 95);
                uint8_t b = (uint8_t)std::clamp(42 + noise * 2, 25, 70);
                drawTilePixel(tx, ty, x, y, r, g, b);
            }
        }
    }
}

void TextureAtlas::paintDirt(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int noise = ((x * 17 + y * 23) % 31) - 15;
            uint8_t r = (uint8_t)std::clamp(98 + noise * 3, 55, 145);
            uint8_t g = (uint8_t)std::clamp(66 + noise * 2, 35, 100);
            uint8_t b = (uint8_t)std::clamp(44 + noise * 2, 20, 75);
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintStone(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int noise = ((x * 19 + y * 29) % 37) - 18;
            uint8_t v = (uint8_t)std::clamp(118 + noise * 3, 70, 175);
            // subtle bluish mystical tint
            drawTilePixel(tx, ty, x, y, v - 4, v - 2, v + 6);
        }
    }
}

void TextureAtlas::paintCobblestone(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool isBorder = (x % 4 == 0) || (y % 4 == 0 && (x / 4) % 2 == (y / 4) % 2);
            if (isBorder) {
                drawTilePixel(tx, ty, x, y, 48, 50, 56);
            } else {
                int noise = ((x * 11 + y * 13) % 23) - 10;
                uint8_t v = (uint8_t)std::clamp(105 + noise * 4, 75, 150);
                drawTilePixel(tx, ty, x, y, v, v, v + 5);
            }
        }
    }
}

void TextureAtlas::paintRuneStone(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            // Dark obsidian slate base
            int noise = ((x * 23 + y * 17) % 19) - 9;
            uint8_t v = (uint8_t)std::clamp(38 + noise * 2, 20, 65);
            uint8_t r = v; uint8_t g = v + 4; uint8_t b = v + 12;

            // Ancient glowing elvish/nordic rune glyph pattern:
            bool isRune = false;
            // Vertical spine
            if (x == 7 && y >= 3 && y <= 12) isRune = true;
            // Diagonal branches
            if ((x == 5 && y == 5) || (x == 6 && y == 4) || (x == 8 && y == 4) || (x == 9 && y == 5)) isRune = true;
            if ((x == 5 && y == 10) || (x == 6 && y == 11) || (x == 8 && y == 11) || (x == 9 && y == 10)) isRune = true;
            // Cross notch
            if (y == 8 && x >= 4 && x <= 10) isRune = true;

            if (isRune) {
                // Radiant cyan / neon turquoise rune glow
                r = 30; g = 240; b = 255;
            } else if (abs(x - 7) <= 2 && y >= 2 && y <= 13) {
                // Rune glow halo
                g += 25; b += 45;
            }

            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintAmethyst(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            // Geometric crystal facets
            int facet = ((x ^ y) * 7 + (x * 3)) % 32;
            uint8_t r = (uint8_t)std::clamp(170 + facet * 3, 110, 255);
            uint8_t g = (uint8_t)std::clamp(45 + facet, 20, 95);
            uint8_t b = (uint8_t)std::clamp(210 + facet * 2, 140, 255);

            // Shimmering specular glint
            if ((x == 4 && y == 4) || (x == 11 && y == 10) || (x == 5 && y == 12)) {
                r = 255; g = 220; b = 255;
            }
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintEmeraldMana(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int facet = ((x * 5 + y * 11) ^ (x * y)) % 29;
            uint8_t r = (uint8_t)std::clamp(25 + facet * 2, 10, 80);
            uint8_t g = (uint8_t)std::clamp(195 + facet * 2, 130, 255);
            uint8_t b = (uint8_t)std::clamp(165 + facet * 3, 100, 245);

            if ((x == 3 && y == 12) || (x == 12 && y == 5)) {
                r = 210; g = 255; b = 245;
            }
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintGoldOre(int tx, int ty) {
    paintStone(tx, ty); // Base stone
    // Sparkly gold nuggets
    int nuggets[6][2] = { {4, 4}, {5, 4}, {10, 7}, {11, 8}, {6, 12}, {7, 12} };
    for (auto& n : nuggets) {
        drawTilePixel(tx, ty, n[0], n[1], 255, 215, 0);
    }
    // High glint
    drawTilePixel(tx, ty, 5, 4, 255, 250, 160);
    drawTilePixel(tx, ty, 11, 8, 255, 250, 160);
}

void TextureAtlas::paintSandstone(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int noise = ((x * 13 + y * 19) % 23) - 10;
            // Layer lines every 4 pixels
            int layer = (y % 4 == 0) ? -18 : 0;
            uint8_t r = (uint8_t)std::clamp(215 + noise * 2 + layer, 150, 245);
            uint8_t g = (uint8_t)std::clamp(185 + noise * 2 + layer, 125, 215);
            uint8_t b = (uint8_t)std::clamp(130 + noise * 2 + layer, 85, 165);
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintHieroglyph(int tx, int ty) {
    paintSandstone(tx, ty);
    // Dark carved ancient hieroglyphic symbols (Eye of Horus & Pharaoh symbol)
    auto carve = [&](int x, int y) {
        drawTilePixel(tx, ty, x, y, 120, 95, 55);
    };
    // Eye outline
    carve(4, 7); carve(5, 6); carve(6, 6); carve(7, 6); carve(8, 6); carve(9, 6); carve(10, 7);
    carve(5, 8); carve(6, 9); carve(7, 9); carve(8, 9); carve(9, 8);
    // Pupil
    drawTilePixel(tx, ty, 7, 7, 80, 50, 25);
    drawTilePixel(tx, ty, 7, 8, 80, 50, 25);
    // Teardrop line
    carve(8, 10); carve(8, 11); carve(7, 12);
}

void TextureAtlas::paintDarkWoodSide(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            // Vertical deep bark crevices
            bool furrow = (x % 5 == 0) || ((x + y * 2) % 9 == 0);
            int noise = ((x * 17 + y * 31) % 19) - 9;
            uint8_t r, g, b;
            if (furrow) {
                r = 35; g = 25; b = 30;
            } else {
                r = (uint8_t)std::clamp(65 + noise * 2, 45, 95);
                g = (uint8_t)std::clamp(48 + noise * 2, 30, 75);
                b = (uint8_t)std::clamp(58 + noise * 2, 40, 85);
            }
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintDarkWoodTop(int tx, int ty) {
    int cx = 7, cy = 7;
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            float dist = std::sqrt((float)((x - cx) * (x - cx) + (y - cy) * (y - cy)));
            if (dist > 6.5f) {
                // Outer bark
                drawTilePixel(tx, ty, x, y, 45, 32, 40);
            } else {
                // Tree rings
                int ring = (int)std::floor(dist) % 2;
                uint8_t v = (ring == 0) ? 95 : 78;
                drawTilePixel(tx, ty, x, y, v + 5, v - 10, v);
            }
        }
    }
}

void TextureAtlas::paintSpookyLeaves(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            // Indigo / violet glowing mystical foliage
            int noise = ((x * 13 + y * 29) % 31) - 15;
            uint8_t r = (uint8_t)std::clamp(90 + noise * 4, 45, 150);
            uint8_t g = (uint8_t)std::clamp(30 + noise * 2, 10, 75);
            uint8_t b = (uint8_t)std::clamp(145 + noise * 4, 90, 210);

            // Bioluminescent glowing spores
            if ((x * 23 + y * 37) % 43 == 0) {
                r = 80; g = 240; b = 255; // Aqua spore
            } else if ((x * 19 + y * 17) % 53 == 0) {
                r = 255; g = 80; b = 220; // Magenta spore
            }

            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintWoodPlanks(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool border = (y % 4 == 0) || ((x % 8 == 0) && ((y / 4) % 2 == 0));
            if (border) {
                drawTilePixel(tx, ty, x, y, 60, 42, 28);
            } else {
                int noise = ((x * 23 + y * 7) % 19) - 9;
                uint8_t r = (uint8_t)std::clamp(145 + noise * 3, 110, 185);
                uint8_t g = (uint8_t)std::clamp(105 + noise * 2, 80, 140);
                uint8_t b = (uint8_t)std::clamp(70 + noise * 2, 50, 95);
                drawTilePixel(tx, ty, x, y, r, g, b);
            }
        }
    }
}

void TextureAtlas::paintDungeonBrick(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool mortar = (y % 4 == 0) || ((y / 4) % 2 == 0 && x % 8 == 0) || ((y / 4) % 2 == 1 && (x + 4) % 8 == 0);
            if (mortar) {
                drawTilePixel(tx, ty, x, y, 35, 38, 42);
            } else {
                int noise = ((x * 17 + y * 23) % 27) - 13;
                uint8_t v = (uint8_t)std::clamp(75 + noise * 2, 45, 110);
                drawTilePixel(tx, ty, x, y, v, v + 2, v + 6);
            }
        }
    }
}

void TextureAtlas::paintGlowLantern(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool frame = (x == 1 || x == 14 || y == 1 || y == 14 || (x == y && x < 4) || (x + y == 15 && x < 4));
            if (frame) {
                // Dark wrought-iron / brass cage
                drawTilePixel(tx, ty, x, y, 75, 60, 30);
            } else {
                // Intense warm solar fire
                int dist = abs(x - 7) + abs(y - 7);
                uint8_t r = 255;
                uint8_t g = (uint8_t)std::clamp(240 - dist * 20, 120, 255);
                uint8_t b = (uint8_t)std::clamp(120 - dist * 18, 20, 180);
                drawTilePixel(tx, ty, x, y, r, g, b);
            }
        }
    }
}

void TextureAtlas::paintWater(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int wave = ((x * 3 + y * 5) % 13);
            uint8_t r = 35 + wave * 2;
            uint8_t g = 110 + wave * 5;
            uint8_t b = 215 + wave * 3;
            drawTilePixel(tx, ty, x, y, r, g, b, 210); // slightly translucent
        }
    }
}

void TextureAtlas::paintSand(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int noise = ((x * 11 + y * 17) % 23) - 11;
            uint8_t r = (uint8_t)std::clamp(225 + noise * 2, 180, 255);
            uint8_t g = (uint8_t)std::clamp(205 + noise * 2, 160, 240);
            uint8_t b = (uint8_t)std::clamp(145 + noise * 2, 105, 180);
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintObsidian(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int noise = ((x * 29 + y * 31) % 37) - 18;
            uint8_t v = (uint8_t)std::clamp(24 + noise, 10, 50);
            uint8_t r = v + 4;
            uint8_t g = v;
            uint8_t b = v + 16; // Deep violet reflection
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintBiomeTile(int tx, int ty, int style) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int n = ((x * 17 + y * 29) % 19) - 9;
            uint8_t r = 0, g = 0, b = 0;
            if (style == 0) { // moss
                r = (uint8_t)std::clamp(63 + n * 2, 30, 90);
                g = (uint8_t)std::clamp(116 + n * 3, 65, 155);
                b = (uint8_t)std::clamp(70 + n * 2, 35, 100);
            } else if (style == 1) { // snow and blue ice flecks
                int shade = std::clamp(225 + n * 2, 175, 255);
                r = (uint8_t)shade; g = (uint8_t)std::clamp(shade + 2, 0, 255); b = (uint8_t)std::clamp(shade + 12, 0, 255);
                if ((x * 7 + y * 11) % 31 == 0) { r = 145; g = 190; b = 225; }
            } else if (style == 2) { // peat
                r = (uint8_t)std::clamp(83 + n * 2, 50, 110);
                g = (uint8_t)std::clamp(65 + n * 2, 35, 90);
                b = (uint8_t)std::clamp(49 + n, 28, 68);
            } else if (style == 3) { // basalt
                int shade = std::clamp(67 + n * 3, 35, 100);
                r = (uint8_t)(shade + 7); g = (uint8_t)(shade - 2); b = (uint8_t)(shade + 4);
                if ((x + y * 3) % 29 == 0) { r = 220; g = 70; b = 25; }
            } else { // moonlit grass
                r = (uint8_t)std::clamp(98 + n * 2, 65, 130);
                g = (uint8_t)std::clamp(156 + n * 3, 100, 200);
                b = (uint8_t)std::clamp(126 + n * 3, 75, 170);
                if ((x * 13 + y * 7) % 41 == 0) { r = 245; g = 225; b = 255; }
            }
            drawTilePixel(tx, ty, x, y, r, g, b);
        }
    }
}

void TextureAtlas::paintBookshelf(int tx, int ty) {
    const uint8_t books[6][3] = {
        {160, 45, 38}, {42, 92, 145}, {190, 145, 48},
        {78, 122, 72}, {112, 58, 132}, {205, 205, 184}
    };
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            if (y == 0 || y == 7 || y == 8 || y == 15) {
                drawTilePixel(tx, ty, x, y, 72, 43, 29);
            } else if (x % 3 != 0) {
                int book = (x / 3 + (y / 3) * 2) % 6;
                drawTilePixel(tx, ty, x, y, books[book][0], books[book][1], books[book][2]);
            } else {
                drawTilePixel(tx, ty, x, y, 50, 31, 24);
            }
        }
    }
}

void TextureAtlas::paintChest(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool edge = x == 0 || x == 15 || y == 0 || y == 15;
            bool band = y == 4 || y == 5 || y == 10 || y == 11;
            bool lock = (x == 7 || x == 8) && y >= 6 && y <= 9;
            if (lock) drawTilePixel(tx, ty, x, y, 250, 211, 82);
            else if (edge) drawTilePixel(tx, ty, x, y, 60, 35, 20);
            else if (band) drawTilePixel(tx, ty, x, y, 105, 84, 45);
            else {
                int grain = ((x * 9 + y * 5) % 15) - 7;
                drawTilePixel(tx, ty, x, y,
                    static_cast<uint8_t>(std::clamp(142 + grain * 3, 115, 170)),
                    static_cast<uint8_t>(std::clamp(83 + grain * 2, 60, 105)),
                    static_cast<uint8_t>(std::clamp(43 + grain, 30, 60)));
            }
        }
    }
}

void TextureAtlas::paintIronChain(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool link = (x >= 6 && x <= 9 && ((y / 4) % 2 == 0)) ||
                        (x >= 5 && x <= 10 && ((y / 4) % 2 == 1) &&
                         (x == 5 || x == 10 || y % 4 == 0 || y % 4 == 3));
            if (link) drawTilePixel(tx, ty, x, y, 178, 183, 193);
            else if (x == 4 || x == 11) drawTilePixel(tx, ty, x, y, 70, 75, 88);
            else drawTilePixel(tx, ty, x, y, 34, 39, 49);
        }
    }
}

void TextureAtlas::paintMarble(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int vein = (x * 5 + y * 9 + (x * y) / 4) % 17;
            int shade = 190 + ((x * 7 + y * 11) % 28) - (vein < 2 ? 52 : 0);
            drawTilePixel(tx, ty, x, y,
                static_cast<uint8_t>(std::clamp(shade, 90, 235)),
                static_cast<uint8_t>(std::clamp(shade - 3, 87, 232)),
                static_cast<uint8_t>(std::clamp(shade + 5, 95, 240)));
        }
    }
}

void TextureAtlas::paintGoldTrim(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool gold = y == 2 || y == 3 || y == 12 || y == 13 ||
                        ((x == 2 || x == 13) && (y < 4 || y > 11));
            if (gold) drawTilePixel(tx, ty, x, y, 226, 174, 54);
            else {
                int shade = 48 + ((x * 13 + y * 7) % 22);
                drawTilePixel(tx, ty, x, y, shade, shade + 2, shade + 9);
            }
        }
    }
}

void TextureAtlas::paintCrystalBrick(int tx, int ty) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            bool mortar = y == 0 || y == 7 || y == 15 ||
                          ((y / 8) == 0 ? x == 0 || x == 8 : x == 4 || x == 12);
            bool shard = ((x + y * 2) % 11 == 0) ||
                         ((x - y + 32) % 13 == 0 && x > 5 && x < 11);
            if (mortar) drawTilePixel(tx, ty, x, y, 38, 30, 55);
            else if (shard) drawTilePixel(tx, ty, x, y, 112, 82, 218);
            else drawTilePixel(tx, ty, x, y, 67, 49, 96);
        }
    }
}

void TextureAtlas::generate() {
    // 0: Air (Transparent)
    // 1: Grass Top
    paintGrassTop(1, 0);
    // 2: Grass Side
    paintGrassSide(2, 0);
    // 3: Dirt
    paintDirt(3, 0);
    // 4: Stone
    paintStone(4, 0);
    // 5: Cobblestone
    paintCobblestone(5, 0);
    // 6: Rune Stone
    paintRuneStone(6, 0);
    // 7: Amethyst Crystal
    paintAmethyst(7, 0);

    // Row 1 (tileY = 1):
    // 8: Emerald Mana
    paintEmeraldMana(0, 1);
    // 9: Gold Ore
    paintGoldOre(1, 1);
    // 10: Sandstone
    paintSandstone(2, 1);
    // 11: Hieroglyph Sandstone
    paintHieroglyph(3, 1);
    // 12: Dark Wood Trunk
    paintDarkWoodSide(4, 1);
    // 13: Dark Wood Top
    paintDarkWoodTop(5, 1);
    // 14: Spooky Leaves
    paintSpookyLeaves(6, 1);
    // 15: Wood Planks
    paintWoodPlanks(7, 1);

    // Row 2 (tileY = 2):
    // 16: Dungeon Brick
    paintDungeonBrick(0, 2);
    // 17: Glow Lantern
    paintGlowLantern(1, 2);
    // 18: Water
    paintWater(2, 2);
    // 19: Sand
    paintSand(3, 2);
    // 20: Obsidian
    paintObsidian(4, 2);

    // 21-25: biome-specific surface materials
    paintBiomeTile(5, 2, 0);
    paintBiomeTile(6, 2, 1);
    paintBiomeTile(7, 2, 2);
    paintBiomeTile(0, 3, 3);
    paintBiomeTile(1, 3, 4);

    // Row 3: castle furnishings and rare masonry (tiles 26-31).
    paintBookshelf(2, 3);
    paintChest(3, 3);
    paintIronChain(4, 3);
    paintMarble(5, 3);
    paintGoldTrim(6, 3);
    paintCrystalBrick(7, 3);

    // Upload to OpenGL
    if (m_textureID == 0) {
        glGenTextures(1, &m_textureID);
    }
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    // Use GL_NEAREST for crisp pixel art!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureAtlas::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void TextureAtlas::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
