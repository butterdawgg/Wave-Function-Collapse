#ifndef GRID_RENDERER_H
#define GRID_RENDERER_H

#include <raylib.h>



class Grid;
class AppCamera;

class GridRenderer final
{
    public:

    GridRenderer();
    ~GridRenderer();

    /* Renders the grid on the screen */
    void DrawGrid(AppCamera* camera);
    /* Sets the Grid that will be rendered */
    void SetGrid(Grid* grid);
    /* Sets the texture that will be used to sample tiles, returns false if texture can't be loaded */
    bool SetSpritesheet(const char* path);
    /* Sets the world position of the grid */
    void SetPosition(Vector2 position);
    /* Sets the world size of the rendered tile */
    void SetTileDstSize(float tileDstSize);
    /* Sets the pixel size of each tile in the spritesheet */
    void SetTileSrcSize(int tileSrcSize);

    private:

    Grid* m_Grid { nullptr };
    Texture2D m_Spritesheet;

    Vector2 m_Position { };
    float m_TileDstSize { 32.0f };
    int m_TileSrcSize { 16 };
};

#endif // !GRID_RENDERER_H