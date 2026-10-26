#pragma once
#include "Common.h"

class TextureAtlas {
public:
    static constexpr int TILE_SIZE = 16;
    static constexpr int TILES_PER_ROW = 8;
    static constexpr int ATLAS_SIZE = TILE_SIZE * TILES_PER_ROW; // 128x128 pixels

    TextureAtlas();
    ~TextureAtlas();

    void generate();
    void bind(unsigned int unit = 0) const;
    void unbind() const;

    // Returns UV bounds [u0, v0, u1, v1] for a tile index (0 to 63)
    void getTileUVs(int tileIndex, float& u0, float& v0, float& u1, float& v1) const;

    GLuint getTextureID() const { return m_textureID; }

private:
    GLuint m_textureID = 0;
    std::vector<uint8_t> m_pixels; // RGBA8

    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void drawTilePixel(int tileX, int tileY, int px, int py, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // Procedural tile painters
    void paintGrassTop(int tileX, int tileY);
    void paintGrassSide(int tileX, int tileY);
    void paintDirt(int tileX, int tileY);
    void paintStone(int tileX, int tileY);
    void paintCobblestone(int tileX, int tileY);
    void paintRuneStone(int tileX, int tileY);
    void paintAmethyst(int tileX, int tileY);
    void paintEmeraldMana(int tileX, int tileY);
    void paintGoldOre(int tileX, int tileY);
    void paintSandstone(int tileX, int tileY);
    void paintHieroglyph(int tileX, int tileY);
    void paintDarkWoodSide(int tileX, int tileY);
    void paintDarkWoodTop(int tileX, int tileY);
    void paintSpookyLeaves(int tileX, int tileY);
    void paintWoodPlanks(int tileX, int tileY);
    void paintDungeonBrick(int tileX, int tileY);
    void paintGlowLantern(int tileX, int tileY);
    void paintWater(int tileX, int tileY);
    void paintSand(int tileX, int tileY);
    void paintObsidian(int tileX, int tileY);
    void paintBiomeTile(int tileX, int tileY, int biomeStyle);
    void paintBookshelf(int tileX, int tileY);
    void paintChest(int tileX, int tileY);
    void paintIronChain(int tileX, int tileY);
    void paintMarble(int tileX, int tileY);
    void paintGoldTrim(int tileX, int tileY);
    void paintCrystalBrick(int tileX, int tileY);
};
