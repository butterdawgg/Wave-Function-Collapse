#include <array>

#include "debug_utils.h"

#include "wfc_trainer.h"
#include "grid.h"



// ---------------
// --- Helpers ---
// ---------------

static std::vector<Color> GetTilePixels(const Image& img, Color* pixels, int tileSize, int row, int col)
{
    std::vector<Color> tilePixels { };
    tilePixels.reserve(tileSize * tileSize);

    const int tileX { col * tileSize };
    const int tileY { row * tileSize };

    for (int x { 0 }; x < tileSize; x++)
    {
        for (int y { 0 }; y < tileSize; y++)
        {
            const int pixelX { tileX + x };
            const int pixelY { tileY + y };

            const int id { pixelY * img.width + pixelX };

            tilePixels.push_back(pixels[id]);
        }
    }

    return tilePixels;
}

static bool AreColorsIdentical(const Color& color1, const Color& color2)
{
    return
        color1.r == color2.r &&
        color1.g == color2.g &&
        color1.b == color2.b &&
        color1.a == color2.a;
}

static bool AreTilesIdentical(const std::vector<Color>& tile1, const std::vector<Color>& tile2)
{
    if (tile1.size() != tile2.size())
    {
        logError("Attempted to compare tiles of different sizes!");

        return false;
    }

    for (int i { 0 }; i < tile1.size(); i++)
    {
        if (!AreColorsIdentical(tile1[i], tile2[i]))
            return false;
    }

    return true;
}

static bool IsTileSingleColor(const std::vector<Color> tilePixels, const Color& color)
{
    for (auto& pixel : tilePixels)
    {
        if (!AreColorsIdentical(pixel, color))
            return false;
    }

    return true;
}



// -------------------
// --- WFC trainer ---
// -------------------

bool WFC_Trainer::BeginTraining(int tileSize, const char* tilesetImagePath, const Color& ignoreColor)
{
    m_IsValid = false;

    m_Settings = WFC_Settings { };
    m_TileSize = tileSize;

    Image tileset { LoadImage(tilesetImagePath) };

    if (tileset.data == NULL)
    {
        logError("Failed to load tileset image at '{}'", tilesetImagePath);

        return false;
    }

    Color* pixels { LoadImageColors(tileset) };
    int tileId { 0 };

    const int rows { tileset.height / tileSize };
    const int cols { tileset.width / tileSize };

    for (int r { 0 }; r < rows; r++)
    {
        for (int c { 0 }; c < cols; c++)
        {
            auto tilePixels { GetTilePixels(tileset, pixels, tileSize, r, c) };

            if (!IsTileSingleColor(tilePixels, ignoreColor))
            {
                m_TilePixels[tileId] = std::move(tilePixels);

                m_Settings.tileIds.push_back(tileId);
            }

            tileId++;
        }
    }

    UnloadImageColors(pixels);
    UnloadImage(tileset);

    m_IsValid = true;
    return true;
}

bool WFC_Trainer::Train(const char* imagePath)
{
    if (!m_IsValid)
    {
        logError("Attempted to call Train() on an invalid WFC_Trainer!");

        return false;
    }

    // Load tilemap image
    Image tilemap { LoadImage(imagePath) };

    if (tilemap.data == NULL)
    {
        logError("Failed to load tilemap image at '{}'", imagePath);

        return false;
    }

    Color* pixels { LoadImageColors(tilemap) };

    // Create grid
    Grid grid { };
    auto& tiles { grid.GetTiles() };
    tiles.Resize(tilemap.height / m_TileSize, tilemap.width / m_TileSize);

    // Parse grid
    for (int r { 0 }; r < tiles.Rows(); r++)
    {
        for (int c { 0 }; c < tiles.Cols(); c++)
        {
            auto tilePixels { GetTilePixels(tilemap, pixels, m_TileSize, r, c) };

            for (auto& [id, tilePx] : m_TilePixels)
            {
                if (AreTilesIdentical(tilePixels, tilePx))
                {
                    tiles(r, c) = id;

                    break;
                }
            }
        }
    }

    // Unload tilemap image
    UnloadImageColors(pixels);
    UnloadImage(tilemap);

    // Add adjacency rules
    for (int r { 0 }; r < tiles.Rows(); r++)
    {
        for (int c { 0 }; c < tiles.Cols(); c++)
        {
            using TileCoord = std::pair<int, int>; // row, col

            TileCoord up { r - 1, c };
            TileCoord right { r, c + 1 };
            TileCoord down { r + 1, c };
            TileCoord left { r, c - 1 };

            const int tileId { tiles(r, c) };

            // Encountered tile -> add to weights
            m_Settings.tileWeights[tileId]++;

            std::array<TileCoord, 4> adjTiles { up, right, down, left };

            for (int dir { 0 }; dir < 4; dir++)
            {
                TileCoord& adjTile = adjTiles[dir];

                if (adjTile.first < 0 || adjTile.first >= tiles.Rows()) // Skip invalid coord
                    continue;

                if (adjTile.second < 0 || adjTile.second >= tiles.Cols()) // Skip invalid coord
                    continue;

                const int adjTileId { tiles(adjTile.first, adjTile.second) };

                if (m_Settings.rules.IsAllowed(tileId, dir, adjTileId)) // Skip already existing rule
                    continue;

                m_Settings.rules.AddRule(tileId, dir, adjTileId);
            }
        }
    }

    return true;
}

const WFC_Settings& WFC_Trainer::GetTrainingData() const
{
    if (!m_IsValid)
    {
        logWarning("Attempted to get training data from an invalid WFC_Trainer! Data may be nonsensical");
    }

    return m_Settings;
}