#include "grid_renderer.h"
#include "wfc/grid.h"
#include "camera.h"
#include "wfc/array2d.h"
#include "debug_utils.h"



GridRenderer::GridRenderer() = default;

GridRenderer::~GridRenderer()
{
    if (m_Spritesheet.id != 0)
        UnloadTexture(m_Spritesheet);
}

void GridRenderer::DrawGrid(AppCamera* camera)
{
    if (!m_Grid || m_Spritesheet.id == 0)
        return;

    Array2d<int>& tiles { m_Grid->GetTiles() };

    const Vector2 camPos { camera->GetPosition() };
    const float camScale { camera->GetScale() };

    // How many tiles fit across the spritesheet width
    const int sheetCols { m_Spritesheet.width / m_TileSrcSize };

    const size_t rows { tiles.Rows() };
    const size_t cols { tiles.Cols() };

    for (size_t row { 0 }; row < rows; ++row)
    {
        for (size_t col { 0 }; col < cols; ++col)
        {
            const int tileId { tiles(row, col) };

            if (tileId == INVALID_TILE_ID)
                continue;

            // Source rect: sample the correct tile from the spritesheet
            const int tileCol { tileId % sheetCols };
            const int tileRow { tileId / sheetCols };

            const Rectangle source
            {
                static_cast<float>(tileCol * m_TileSrcSize),
                static_cast<float>(tileRow * m_TileSrcSize),
                static_cast<float>(m_TileSrcSize),
                static_cast<float>(m_TileSrcSize)
            };

            // World position of this tile
            const float worldX { m_Position.x + static_cast<float>(col) * m_TileDstSize };
            const float worldY { m_Position.y + static_cast<float>(row) * m_TileDstSize };

            const float screenW { static_cast<float>(GetScreenWidth()) };
            const float screenH { static_cast<float>(GetScreenHeight()) };

            // Screen position: translate by camera (center-origin), then scale
            const float screenX { (worldX - camPos.x) * camScale + screenW * 0.5f };
            const float screenY { (worldY - camPos.y) * camScale + screenH * 0.5f };

            const Rectangle dest
            {
                screenX,
                screenY,
                m_TileDstSize * camScale,
                m_TileDstSize * camScale
            };

            DrawTexturePro(m_Spritesheet, source, dest, { 0.0f, 0.0f }, 0.0f, WHITE);
        }
    }
}

void GridRenderer::SetGrid(Grid* grid)
{
    m_Grid = grid;
}

bool GridRenderer::SetSpritesheet(const char* path)
{
    Texture2D tex { LoadTexture(path) };

    if (tex.id == 0)
    {
        logError("Failed to load spritesheet texture at '{}'", path);

        return false;
    }

    if (m_Spritesheet.id != 0)
        UnloadTexture(m_Spritesheet);

    m_Spritesheet = tex;
    return true;
}

void GridRenderer::SetPosition(Vector2 position)
{
    m_Position = position;
}

void GridRenderer::SetTileDstSize(float tileDstSize)
{
    m_TileDstSize = tileDstSize;
}

void GridRenderer::SetTileSrcSize(int tileSrcSize)
{
    m_TileSrcSize = tileSrcSize;
}