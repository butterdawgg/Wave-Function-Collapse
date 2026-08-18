#ifndef GRID_H
#define GRID_H

#include "array2d.h"



static constexpr int INVALID_TILE_ID { -1 };

class Grid final
{
    public:

    Grid() = default;
    ~Grid() = default;

    Array2d<int>& GetTiles();

    private:

    Array2d<int> m_Tiles;
};

#endif // !GRID_H