#include <algorithm>
#include <random>
#include <stack>

#include "wfc_solver.h"
#include "grid.h"



// ----------------------
// --- AdjacencyRules ---
// ----------------------

void AdjacencyRules::AddRule(int tileId, int direction, int neighbourId)
{
    auto& dirs { rules[tileId] };

    if (dirs.size() < 4)
        dirs.resize(4);

    auto& allowed { dirs[direction] };

    if (std::find(allowed.begin(), allowed.end(), neighbourId) == allowed.end())
        allowed.push_back(neighbourId);
}

bool AdjacencyRules::IsAllowed(int tileId, int direction, int neighbourId) const
{
    auto it { rules.find(tileId) };

    if (it == rules.end())
        return false;

    const auto& dirs { it->second };

    if (direction >= static_cast<int>(dirs.size()))
        return false;

    const auto& allowed { dirs[direction] };

    return std::find(allowed.begin(), allowed.end(), neighbourId) != allowed.end();
}



// ------------------------------
// --- Wave function collapse ---
// ------------------------------

WFC_Solver::WFC_Solver(Grid& grid, const WFC_Settings& settings)
    : m_Grid(grid), m_Settings(settings)
{ }

void WFC_Solver::Initialize()
{
    m_Cells = Array2d<std::vector<int>>(m_Grid.GetTiles().Rows(), m_Grid.GetTiles().Cols());

    m_HeapVersion.assign(m_Cells.Rows() * m_Cells.Cols(), 0);
    m_CellVersion.assign(m_Cells.Rows() * m_Cells.Cols(), 0);

    // Clear the heap
    m_Heap = { };

    for (int r { 0 }; r < m_Cells.Rows(); r++)
    {
        for (int c { 0 }; c < m_Cells.Cols(); c++)
        {
            m_Cells(r, c) = m_Settings.tileIds;

            m_Heap.push({ ShannonEntropy(r, c), m_Cells.Id(r, c) });
        }
    }
}

bool WFC_Solver::IsCollapsed(int row, int col) const
{
    return m_Cells(row, col).size() == 1;
}

/* Returns false if all remaining cells are already collapsed */
bool WFC_Solver::PickLowestEntropyCell(int& outRow, int& outCol)
{
    while (!m_Heap.empty())
    {
        auto [entropy, index] { m_Heap.top() };
        m_Heap.pop();

        // Stale entry check: if versions don't match this entry was superseded
        if (m_HeapVersion[index] != m_CellVersion[index])
            continue;

        const int row { index / static_cast<int>(m_Cells.Cols()) };
        const int col { index % static_cast<int>(m_Cells.Cols()) };

        if (IsCollapsed(row, col))
            continue;

        outRow = row;
        outCol = col;

        return true;
    }

    return false; // All collapsed
}

void WFC_Solver::PushToHeap(int row, int col)
{
    const int index { static_cast<int>(m_Cells.Id(row, col)) };

    // Increment cell version to invalidate any existing heap entries for this cell
    ++m_CellVersion[index];
    m_HeapVersion[index] = m_CellVersion[index];

    m_Heap.push({ ShannonEntropy(row, col), index });
}

int WFC_Solver::CollapseCell(int row, int col)
{
    static std::mt19937 rng(std::random_device { }());

    auto& possible { m_Cells(row, col) };

    std::vector<float> weights;
    weights.reserve(possible.size());

    for (int tileId : possible)
    {
        auto it { m_Settings.tileWeights.find(tileId) };
        weights.push_back(it != m_Settings.tileWeights.end() ? it->second : 1.0f);
    }

    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    const int chosen { possible[dist(rng)] };
    possible = { chosen };

    ++m_CellVersion[m_Cells.Id(row, col)];

    return chosen;
}

bool WFC_Solver::Propagate(int startRow, int startCol)
{
    std::stack<std::pair<int, int>> stack;
    stack.push({ startRow, startCol });

    while (!stack.empty())
    {
        auto [row, col] { stack.top() };
        stack.pop();

        const auto& currentPossible { m_Cells(row, col) };

        for (int dir = 0; dir < 4; ++dir)
        {
            const int nr { row + DR[dir] };
            const int nc { col + DC[dir] };

            if (nr < 0 || nr >= m_Cells.Rows() || nc < 0 || nc >= m_Cells.Cols())
                continue;

            auto& neighbourPossible { m_Cells(nr, nc) };
            const int before { static_cast<int>(neighbourPossible.size()) };

            neighbourPossible.erase(
                std::remove_if(neighbourPossible.begin(), neighbourPossible.end(),
                    [&](int neighbourTile)
                    {
                        return std::none_of(currentPossible.begin(), currentPossible.end(),
                            [&](int currentTile)
                            {
                                return m_Settings.rules.IsAllowed(currentTile, dir, neighbourTile);
                            });
                    }),
                neighbourPossible.end()
            );

            if (neighbourPossible.empty())
                return false;

            if (static_cast<int>(neighbourPossible.size()) < before)
            {
                PushToHeap(nr, nc); // Update heap with new lower entropy
                stack.push({ nr, nc });
            }
        }
    }

    return true;
}

float WFC_Solver::ShannonEntropy(int row, int col) const
{
    const auto& possible { m_Cells(row, col) };

    float sumWeights { 0.0f };
    float sumWlogW { 0.0f };

    for (int tileId : possible)
    {
        auto it { m_Settings.tileWeights.find(tileId) };
        float w { it != m_Settings.tileWeights.end() ? it->second : 1.0f };

        sumWeights += w;
        sumWlogW += w * std::log(w);
    }

    // H = log(sum(w)) - sum(w * log(w)) / sum(w)
    return std::log(sumWeights) - (sumWlogW / sumWeights);
}


bool WFC_Solver::Solve()
{
    Initialize();

    while (true)
    {
        int row, col;

        if (!PickLowestEntropyCell(row, col))
            break; // All cells collapsed — done

        CollapseCell(row, col);

        if (!Propagate(row, col))
            return false;
    }

    Array2d<int>& tiles { m_Grid.GetTiles() };
    tiles.Resize(m_Cells.Rows(), m_Cells.Cols());

    for (int r = 0; r < m_Cells.Rows(); ++r)
    {
        for (int c = 0; c < m_Cells.Cols(); ++c)
        {
            tiles(r, c) = m_Cells(r, c).front();
        }
    }

    return true;
}