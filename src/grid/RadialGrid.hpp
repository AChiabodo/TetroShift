#pragma once
#include "IGrid.hpp"
#include <vector>

namespace TetroShift {

class RadialGrid : public IGrid {
public:
    RadialGrid(int numSectors = 16, int numRings = 12, int bufferRings = 3);
    ~RadialGrid() override = default;

    void Initialize(int width, int height, int bufferHeight = 3) override;
    void Clear() override;

    [[nodiscard]] int GetWidth() const noexcept override { return m_numSectors; }
    [[nodiscard]] int GetHeight() const noexcept override { return m_numRings; }
    [[nodiscard]] int GetTotalHeight() const noexcept override { return m_numRings + m_bufferRings; }
    [[nodiscard]] GridGeometry GetGeometryType() const noexcept override { return GridGeometry::Radial; }

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

    void ExplodeArea(const GridCoord& center, int radius, std::vector<GridCoord>& outDestroyed);

    [[nodiscard]] float GetInnerRadius() const noexcept { return m_innerRadius; }
    [[nodiscard]] float GetOuterRadius() const noexcept { return m_outerRadius; }

private:
    [[nodiscard]] int CoordToIndex(const GridCoord& coord) const noexcept;
    [[nodiscard]] int NormalizeSector(int x) const noexcept;

    int m_numSectors = 16;
    int m_numRings = 12;
    int m_bufferRings = 3;
    float m_innerRadius = 55.0f;
    float m_outerRadius = 310.0f;
    float m_pulseTimer = 0.0f;

    std::vector<Cell> m_cells;
    Cell m_emptyCell;
};

} // namespace TetroShift
