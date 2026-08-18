#ifndef WFC_H
#define WFC_H

#include <vector>
#include <unordered_map>
#include <queue>

#include "array2d.h"



class Grid;

inline constexpr int DIR_U { 0 };
inline constexpr int DIR_R { 1 };
inline constexpr int DIR_D { 2 };
inline constexpr int DIR_L { 3 };

struct AdjacencyRules final
{
    // Directions: 0 = Up, 1 = Right, 2 = Down, 3 = Left
    void AddRule(int tileId, int direction, int neighbourId);
    bool IsAllowed(int tileId, int direction, int neighbourId) const;

    std::unordered_map<int, std::vector<std::vector<int>>> rules;
};

struct WFC_Settings final
{
    AdjacencyRules rules;
    std::vector<int> tileIds;
    std::unordered_map<int, float> tileWeights;
};

class WFC_Solver final
{
    public:

    WFC_Solver(Grid& grid, const WFC_Settings& settings);

    bool Solve();

    private:

    void Initialize();
    bool Propagate(int row, int col);
    bool PickLowestEntropyCell(int& outRow, int& outCol);
    int CollapseCell(int row, int col);
    bool IsCollapsed(int row, int col) const;
    void PushToHeap(int row, int col);
    float ShannonEntropy(int row, int col) const;

    Grid& m_Grid;
    WFC_Settings m_Settings;

    Array2d<std::vector<int>> m_Cells;

    // Min-heap: (entropy, cellIndex)
    using HeapEntry = std::pair<float, int>;
    std::priority_queue<HeapEntry, std::vector<HeapEntry>, std::greater<HeapEntry>> m_Heap;

    // Tracks whether a cell's heap entry is stale (lazy deletion)
    std::vector<int> m_HeapVersion;
    std::vector<int> m_CellVersion;

    static constexpr int DR[4] = { -1, 0, 1, 0 };
    static constexpr int DC[4] = { 0, 1, 0, -1 };
};

#endif // !WFC_H