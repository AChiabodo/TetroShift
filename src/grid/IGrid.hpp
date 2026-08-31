#pragma once
#include "GridCoord.hpp"
#include "Cell.hpp"
#include "GeometryTypes.hpp"
#include <vector>
#include <string>
#include <raylib.h>

namespace TetroShift {

struct LineClearResult {
    int linesCount = 0;
    std::vector<int> rowsCleared;
    std::vector<GridCoord> clearedCells;
    std::vector<GridCoord> bombExplosions;
    int coinsGenerated = 0;
    float scoreMultiplier = 1.0f;
    bool isTetris = false;
};

class IGrid {
public:
    virtual ~IGrid() = default;

    virtual void Initialize(int width, int height, int bufferHeight = 4) = 0;
    virtual void Clear() = 0;

    [[nodiscard]] virtual int GetWidth() const noexcept = 0;
    [[nodiscard]] virtual int GetHeight() const noexcept = 0;
    [[nodiscard]] virtual int GetTotalHeight() const noexcept = 0;
    [[nodiscard]] virtual GridGeometry GetGeometryType() const noexcept = 0;

    [[nodiscard]] virtual bool IsValidCoord(const GridCoord& coord) const noexcept = 0;
    [[nodiscard]] virtual bool IsCellOccupied(const GridCoord& coord) const noexcept = 0;
    [[nodiscard]] virtual const Cell& GetCell(const GridCoord& coord) const = 0;
    virtual void SetCell(const GridCoord& coord, const Cell& cell) = 0;

    virtual LineClearResult CheckAndClearLines() = 0;
    virtual void Update(float dt) = 0;

    [[nodiscard]] virtual Vector2 CoordToWorld(const GridCoord& coord, Vector2 gridOrigin, float cellSize) const noexcept = 0;
    virtual void Render(Vector2 gridOrigin, float cellSize, bool showDebugHitbox = false) const = 0;

    virtual void PushGarbageRow(int holeCol, CellType type = CellType::Solid, Color color = DARKGRAY) = 0;
    virtual void VaporizeTopRows(int count) = 0;
    virtual void VaporizeBottomRow() = 0;
    virtual void CollapseFloatingCells() = 0;

    [[nodiscard]] virtual std::vector<std::string> Serialize() const = 0;
    virtual void Deserialize(const std::vector<std::string>& data) = 0;
};

} // namespace TetroShift
