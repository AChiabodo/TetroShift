#pragma once
#include "IGrid.hpp"
#include <vector>

namespace TetroShift {

class OrthogonalGrid : public IGrid {
public:
    OrthogonalGrid(int width = 10, int height = 20, int bufferHeight = 4);
    ~OrthogonalGrid() override = default;

    void Initialize(int width, int height, int bufferHeight = 4) override;
    void Clear() override;

    [[nodiscard]] int GetWidth() const noexcept override { return m_width; }
    [[nodiscard]] int GetHeight() const noexcept override { return m_height; }
    [[nodiscard]] int GetTotalHeight() const noexcept override { return m_height + m_bufferHeight; }
    [[nodiscard]] GridGeometry GetGeometryType() const noexcept override { return GridGeometry::Orthogonal; }

    [[nodiscard]] bool IsValidCoord(const GridCoord& coord) const noexcept override;
    [[nodiscard]] bool IsCellOccupied(const GridCoord& coord) const noexcept override;
    [[nodiscard]] const Cell& GetCell(const GridCoord& coord) const override;
    void SetCell(const GridCoord& coord, const Cell& cell) override;

    LineClearResult CheckAndClearLines() override;
    void Update(float dt) override;

    [[nodiscard]] Vector2 CoordToWorld(const GridCoord& coord, Vector2 gridOrigin, float cellSize) const noexcept override;
    void Render(Vector2 gridOrigin, float cellSize, bool showDebugHitbox = false) const override;

    // Special modifier actions
    void ApplyHorizontalMagneticPull(bool toRight);
    void ExplodeArea(const GridCoord& center, int radius, std::vector<GridCoord>& outDestroyed);

private:
    [[nodiscard]] int CoordToIndex(const GridCoord& coord) const noexcept;

    int m_width = 10;
    int m_height = 20;
    int m_bufferHeight = 4;
    std::vector<Cell> m_cells;
    Cell m_emptyCell;
};

} // namespace TetroShift
