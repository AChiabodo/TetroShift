#pragma once
#include "IGrid.hpp"
#include <vector>

namespace TetroShift {

class HexagonalGrid : public IGrid {
public:
    HexagonalGrid(int width = 10, int height = 18, int bufferHeight = 4);
    ~HexagonalGrid() override = default;

    void Initialize(int width, int height, int bufferHeight = 4) override;
    void Clear() override;

    [[nodiscard]] int GetWidth() const noexcept override { return m_width; }
    [[nodiscard]] int GetHeight() const noexcept override { return m_height; }
    [[nodiscard]] int GetTotalHeight() const noexcept override { return m_height + m_bufferHeight; }
    [[nodiscard]] GridGeometry GetGeometryType() const noexcept override { return GridGeometry::Hexagonal; }

    [[nodiscard]] bool IsValidCoord(const GridCoord& coord) const noexcept override;
    [[nodiscard]] bool IsCellOccupied(const GridCoord& coord) const noexcept override;
    [[nodiscard]] const Cell& GetCell(const GridCoord& coord) const override;
    void SetCell(const GridCoord& coord, const Cell& cell) override;

    LineClearResult CheckAndClearLines() override;
    void Update(float dt) override;

    [[nodiscard]] Vector2 CoordToWorld(const GridCoord& coord, Vector2 gridOrigin, float cellSize) const noexcept override;
    void Render(Vector2 gridOrigin, float cellSize, bool showDebugHitbox = false) const override;

    void PushGarbageRow(int holeCol, CellType type = CellType::Solid, Color color = DARKGRAY) override;
    void VaporizeTopRows(int count) override;
    void VaporizeBottomRow() override;
    void CollapseFloatingCells() override;

    [[nodiscard]] std::vector<std::string> Serialize() const override;
    void Deserialize(const std::vector<std::string>& data) override;

    bool UpdateSandPhysics();
    void ExplodeArea(const GridCoord& center, int radius, std::vector<GridCoord>& outDestroyed);

    // Hexagonal neighbor offsets
    [[nodiscard]] static std::vector<GridCoord> GetHexNeighbors(const GridCoord& coord) noexcept;

private:
    [[nodiscard]] int CoordToIndex(const GridCoord& coord) const noexcept;

    int m_width = 10;
    int m_height = 18;
    int m_bufferHeight = 4;
    float m_sandTimer = 0.0f;
    std::vector<Cell> m_cells;
    Cell m_emptyCell;
};

} // namespace TetroShift
