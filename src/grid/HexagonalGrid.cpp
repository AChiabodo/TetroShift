#include "HexagonalGrid.hpp"
#include "core/Constants.hpp"
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace TetroShift {

HexagonalGrid::HexagonalGrid(int width, int height, int bufferHeight)
    : m_width(width), m_height(height), m_bufferHeight(bufferHeight) {
    Initialize(width, height, bufferHeight);
}

void HexagonalGrid::Initialize(int width, int height, int bufferHeight) {
    m_width = width;
    m_height = height;
    m_bufferHeight = bufferHeight;
    m_cells.resize(m_width * (m_height + m_bufferHeight));
    Clear();
}

void HexagonalGrid::Clear() {
    for (auto& cell : m_cells) {
        cell.Clear();
    }
}

int HexagonalGrid::CoordToIndex(const GridCoord& coord) const noexcept {
    const int effectiveY = coord.y + m_bufferHeight;
    if (coord.x < 0 || coord.x >= m_width || effectiveY < 0 || effectiveY >= (m_height + m_bufferHeight)) {
        return -1;
    }
    return effectiveY * m_width + coord.x;
}

bool HexagonalGrid::IsValidCoord(const GridCoord& coord) const noexcept {
    const int effectiveY = coord.y + m_bufferHeight;
    return coord.x >= 0 && coord.x < m_width && effectiveY >= 0 && effectiveY < (m_height + m_bufferHeight);
}

bool HexagonalGrid::IsCellOccupied(const GridCoord& coord) const noexcept {
    const int idx = CoordToIndex(coord);
    if (idx < 0) {
        // Out of bounds
        if (coord.x < 0 || coord.x >= m_width || coord.y >= m_height) {
            return true;
        }
        return false;
    }
    return m_cells[idx].IsSolid();
}

const Cell& HexagonalGrid::GetCell(const GridCoord& coord) const {
    const int idx = CoordToIndex(coord);
    if (idx < 0) {
        return m_emptyCell;
    }
    return m_cells[idx];
}

void HexagonalGrid::SetCell(const GridCoord& coord, const Cell& cell) {
    const int idx = CoordToIndex(coord);
    if (idx >= 0 && idx < static_cast<int>(m_cells.size())) {
        m_cells[idx] = cell;
    }
}

std::vector<GridCoord> HexagonalGrid::GetHexNeighbors(const GridCoord& coord) noexcept {
    std::vector<GridCoord> neighbors;
    neighbors.reserve(6);
    neighbors.push_back({ coord.x - 1, coord.y });
    neighbors.push_back({ coord.x + 1, coord.y });

    if ((coord.y & 1) == 0) {
        // Even row
        neighbors.push_back({ coord.x - 1, coord.y - 1 });
        neighbors.push_back({ coord.x, coord.y - 1 });
        neighbors.push_back({ coord.x - 1, coord.y + 1 });
        neighbors.push_back({ coord.x, coord.y + 1 });
    } else {
        // Odd row
        neighbors.push_back({ coord.x, coord.y - 1 });
        neighbors.push_back({ coord.x + 1, coord.y - 1 });
        neighbors.push_back({ coord.x, coord.y + 1 });
        neighbors.push_back({ coord.x + 1, coord.y + 1 });
    }
    return neighbors;
}

void HexagonalGrid::Update(float dt) {
    m_sandTimer += dt;
    while (m_sandTimer >= SAND_TICK_INTERVAL) {
        m_sandTimer -= SAND_TICK_INTERVAL;
        UpdateSandPhysics();
    }

    for (auto& cell : m_cells) {
        if (cell.flashTimer > 0.0f) {
            cell.flashTimer = std::max(0.0f, cell.flashTimer - dt * 3.0f);
        }
        if (std::abs(cell.squishOffset) > 0.01f) {
            cell.squishOffset *= std::exp(-8.0f * dt);
        } else {
            cell.squishOffset = 0.0f;
        }
    }
}

bool HexagonalGrid::UpdateSandPhysics() {
    bool movedAny = false;
    // Scan bottom-up
    for (int y = m_height - 2; y >= 0; --y) {
        for (int x = 0; x < m_width; ++x) {
            const int idx = CoordToIndex({ x, y });
            if (idx < 0) continue;

            if (m_cells[idx].IsSolid() && m_cells[idx].type == CellType::Sand) {
                // Downward neighbor options on hex grid
                GridCoord downLeft = ((y & 1) == 0) ? GridCoord{ x - 1, y + 1 } : GridCoord{ x, y + 1 };
                GridCoord downRight = ((y & 1) == 0) ? GridCoord{ x, y + 1 } : GridCoord{ x + 1, y + 1 };

                // Try straight drop if available or alternate diagonal rolls
                if (downLeft.x >= 0 && downLeft.x < m_width && downLeft.y < m_height && !IsCellOccupied(downLeft)) {
                    int targetIdx = CoordToIndex(downLeft);
                    m_cells[targetIdx] = m_cells[idx];
                    m_cells[idx].Clear();
                    movedAny = true;
                } else if (downRight.x >= 0 && downRight.x < m_width && downRight.y < m_height && !IsCellOccupied(downRight)) {
                    int targetIdx = CoordToIndex(downRight);
                    m_cells[targetIdx] = m_cells[idx];
                    m_cells[idx].Clear();
                    movedAny = true;
                }
            }
        }
    }
    return movedAny;
}

void HexagonalGrid::ExplodeArea(const GridCoord& center, int radius, std::vector<GridCoord>& outDestroyed) {
    std::vector<GridCoord> queue;
    std::unordered_set<int> visited;

    queue.push_back(center);
    visited.insert(CoordToIndex(center));

    for (int r = 0; r < radius; ++r) {
        size_t levelSize = queue.size();
        for (size_t i = 0; i < levelSize; ++i) {
            GridCoord curr = queue[i];
            auto neighbors = GetHexNeighbors(curr);
            for (const auto& n : neighbors) {
                if (IsValidCoord(n) && n.y >= 0) {
                    int nIdx = CoordToIndex(n);
                    if (nIdx >= 0 && visited.find(nIdx) == visited.end()) {
                        visited.insert(nIdx);
                        queue.push_back(n);
                    }
                }
            }
        }
    }

    std::vector<GridCoord> secondaryBombs;
    for (const auto& c : queue) {
        if (IsValidCoord(c) && c.y >= 0) {
            int idx = CoordToIndex(c);
            if (idx >= 0 && m_cells[idx].IsSolid()) {
                outDestroyed.push_back(c);
                if (m_cells[idx].type == CellType::Bomb && (c.x != center.x || c.y != center.y)) {
                    secondaryBombs.push_back(c);
                }
                m_cells[idx].Clear();
            }
        }
    }

    for (const auto& sb : secondaryBombs) {
        ExplodeArea(sb, radius, outDestroyed);
    }
}

LineClearResult HexagonalGrid::CheckAndClearLines() {
    LineClearResult result;
    std::vector<int> fullRows;

    // 1. Horizontal Rows Check
    for (int y = m_height - 1; y >= 0; --y) {
        bool isFull = true;
        for (int x = 0; x < m_width; ++x) {
            const int idx = CoordToIndex({ x, y });
            if (idx < 0 || !m_cells[idx].IsSolid()) {
                isFull = false;
                break;
            }
        }
        if (isFull) {
            fullRows.push_back(y);
        }
    }

    if (fullRows.empty()) {
        return result;
    }

    result.linesCount = static_cast<int>(fullRows.size());
    result.rowsCleared = fullRows;
    result.isTetris = (result.linesCount >= 4);

    // Identify cleared cells & bomb/gold effects
    for (int row : fullRows) {
        for (int x = 0; x < m_width; ++x) {
            GridCoord c{ x, row };
            result.clearedCells.push_back(c);

            const int idx = CoordToIndex(c);
            if (idx >= 0) {
                if (m_cells[idx].type == CellType::Gold) {
                    result.coinsGenerated += 5;
                } else if (m_cells[idx].type == CellType::Bomb) {
                    result.bombExplosions.push_back(c);
                }
            }
        }
    }

    // Process bomb explosions across hex graph
    for (const auto& bombCoord : result.bombExplosions) {
        std::vector<GridCoord> destroyedByBlast;
        ExplodeArea(bombCoord, 2, destroyedByBlast);
        for (const auto& d : destroyedByBlast) {
            result.clearedCells.push_back(d);
        }
    }

    // Clear rows and collapse downward
    for (int clearedRow : fullRows) {
        for (int y = clearedRow; y > -m_bufferHeight; --y) {
            for (int x = 0; x < m_width; ++x) {
                int currIdx = CoordToIndex({ x, y });
                int aboveIdx = CoordToIndex({ x, y - 1 });
                if (currIdx >= 0) {
                    if (aboveIdx >= 0) {
                        m_cells[currIdx] = m_cells[aboveIdx];
                    } else {
                        m_cells[currIdx].Clear();
                    }
                }
            }
        }
        for (int x = 0; x < m_width; ++x) {
            int topIdx = CoordToIndex({ x, -m_bufferHeight });
            if (topIdx >= 0) {
                m_cells[topIdx].Clear();
            }
        }
    }

    return result;
}

Vector2 HexagonalGrid::CoordToWorld(const GridCoord& coord, Vector2 gridOrigin, float /*cellSize*/) const noexcept {
    const float spacingX = 35.0f;
    const float spacingY = 32.0f;
    const float startX = gridOrigin.x - 20.0f;
    const float rowOffset = ((coord.y & 1) != 0) ? (spacingX * 0.5f) : 0.0f;

    return {
        startX + static_cast<float>(coord.x) * spacingX + rowOffset,
        gridOrigin.y + static_cast<float>(coord.y) * spacingY
    };
}

void HexagonalGrid::Render(Vector2 gridOrigin, float cellSize, bool showDebugHitbox) const {
    const float radius = 19.5f;
    const float spacingX = 35.0f;
    const float spacingY = 32.0f;
    const float startX = gridOrigin.x - 20.0f;
    const float totalWidth = static_cast<float>(m_width) * spacingX + spacingX * 0.5f;
    const float totalHeight = static_cast<float>(m_height) * spacingY;

    // Draw Honeycomb Playfield Background Container
    DrawRectangleRounded({ startX - 4.0f, gridOrigin.y - 4.0f, totalWidth + 8.0f, totalHeight + 8.0f }, 0.04f, 6, Colors::GridBg);
    DrawRectangleLinesEx({ startX - 4.0f, gridOrigin.y - 4.0f, totalWidth + 8.0f, totalHeight + 8.0f }, 1.5f, Colors::BgPanelBorder);

    // Draw Sub-grid Honeycomb Cells (faint neon wires)
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            Vector2 pos = CoordToWorld({ x, y }, gridOrigin, cellSize);
            Vector2 center = { pos.x + cellSize * 0.5f, pos.y + cellSize * 0.5f };
            DrawPolyLinesEx(center, 6, radius, 30.0f, 1.0f, Colors::GridLine);
        }
    }

    // Draw Locked Solid Hexagonal Cells
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const int idx = CoordToIndex({ x, y });
            if (idx < 0) continue;

            const Cell& cell = m_cells[idx];
            if (!cell.IsSolid()) continue;

            Vector2 pos = CoordToWorld({ x, y }, gridOrigin, cellSize);
            Vector2 center = { pos.x + cellSize * 0.5f, pos.y + cellSize * 0.5f + cell.squishOffset };

            Color drawColor = cell.color;
            if (cell.flashTimer > 0.0f) {
                drawColor = ColorAlphaBlend(drawColor, WHITE, Fade(WHITE, cell.flashTimer));
            }

            // Hexagon Body
            DrawPoly(center, 6, radius - 1.5f, 30.0f, drawColor);
            DrawPolyLinesEx(center, 6, radius - 1.5f, 30.0f, 1.5f, ColorAlphaBlend(drawColor, WHITE, Fade(WHITE, 0.4f)));

            // Inner bevel
            DrawPolyLinesEx(center, 6, radius * 0.65f, 30.0f, 1.0f, Fade(BLACK, 0.35f));

            // Special cell icons
            if (cell.type == CellType::Bomb) {
                DrawCircle(static_cast<int>(center.x), static_cast<int>(center.y), radius * 0.35f, RED);
                DrawCircle(static_cast<int>(center.x), static_cast<int>(center.y), radius * 0.18f, YELLOW);
            } else if (cell.type == CellType::Gold) {
                DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius * 0.38f, GOLD);
            } else if (cell.type == CellType::Jelly) {
                DrawCircle(static_cast<int>(center.x), static_cast<int>(center.y), radius * 0.3f, Fade(WHITE, 0.5f));
            } else if (cell.type == CellType::Sand) {
                DrawCircle(static_cast<int>(center.x - 3.0f), static_cast<int>(center.y - 3.0f), 1.5f, Fade(WHITE, 0.6f));
                DrawCircle(static_cast<int>(center.x + 3.0f), static_cast<int>(center.y + 3.0f), 1.5f, Fade(DARKGRAY, 0.4f));
            }

            if (showDebugHitbox) {
                DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radius, MAGENTA);
            }
        }
    }
}

void HexagonalGrid::PushGarbageRow(int holeCol, CellType type, Color color) {
    // Shift all cells up by 1
    for (int y = -m_bufferHeight; y < m_height - 1; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int currIdx = CoordToIndex({ x, y });
            int belowIdx = CoordToIndex({ x, y + 1 });
            if (currIdx >= 0 && belowIdx >= 0) {
                m_cells[currIdx] = m_cells[belowIdx];
            }
        }
    }

    // Fill bottom row with garbage except holeCol
    for (int x = 0; x < m_width; ++x) {
        int bIdx = CoordToIndex({ x, m_height - 1 });
        if (bIdx >= 0) {
            if (x == holeCol) {
                m_cells[bIdx].Clear();
            } else {
                m_cells[bIdx].type = type;
                m_cells[bIdx].color = color;
                m_cells[bIdx].isLocked = true;
                m_cells[bIdx].flashTimer = 0.3f;
            }
        }
    }
}

void HexagonalGrid::VaporizeTopRows(int count) {
    for (int y = 0; y < count && y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int idx = CoordToIndex({ x, y });
            if (idx >= 0) {
                m_cells[idx].Clear();
            }
        }
    }
}

void HexagonalGrid::VaporizeBottomRow() {
    for (int x = 0; x < m_width; ++x) {
        int idx = CoordToIndex({ x, m_height - 1 });
        if (idx >= 0) {
            m_cells[idx].Clear();
        }
    }
}

void HexagonalGrid::CollapseFloatingCells() {
    for (int y = m_height - 2; y >= 0; --y) {
        for (int x = 0; x < m_width; ++x) {
            int idx = CoordToIndex({ x, y });
            if (idx >= 0 && m_cells[idx].IsSolid()) {
                GridCoord downLeft = ((y & 1) == 0) ? GridCoord{ x - 1, y + 1 } : GridCoord{ x, y + 1 };
                GridCoord downRight = ((y & 1) == 0) ? GridCoord{ x, y + 1 } : GridCoord{ x + 1, y + 1 };
                if (!IsCellOccupied(downLeft) && !IsCellOccupied(downRight)) {
                    int targetIdx = CoordToIndex(downLeft.x >= 0 ? downLeft : downRight);
                    if (targetIdx >= 0) {
                        m_cells[targetIdx] = m_cells[idx];
                        m_cells[idx].Clear();
                    }
                }
            }
        }
    }
}

std::vector<std::string> HexagonalGrid::Serialize() const {
    std::vector<std::string> rows;
    rows.reserve(m_height);
    for (int y = 0; y < m_height; ++y) {
        std::string rowStr;
        for (int x = 0; x < m_width; ++x) {
            const int idx = CoordToIndex({ x, y });
            if (idx >= 0 && m_cells[idx].IsSolid()) {
                rowStr += static_cast<char>('0' + static_cast<int>(m_cells[idx].type));
            } else {
                rowStr += '.';
            }
        }
        rows.push_back(rowStr);
    }
    return rows;
}

void HexagonalGrid::Deserialize(const std::vector<std::string>& data) {
    Clear();
    for (int y = 0; y < static_cast<int>(data.size()) && y < m_height; ++y) {
        const std::string& row = data[y];
        for (int x = 0; x < static_cast<int>(row.size()) && x < m_width; ++x) {
            if (row[x] != '.') {
                int t = row[x] - '0';
                Cell c;
                c.type = static_cast<CellType>(t);
                c.color = (c.type == CellType::Bomb) ? Colors::PieceBomb :
                          (c.type == CellType::Gold) ? Colors::PieceGold :
                          (c.type == CellType::Jelly) ? Colors::PieceJelly :
                          (c.type == CellType::Sand) ? Colors::PieceSand : Colors::PieceI;
                c.isLocked = true;
                SetCell({ x, y }, c);
            }
        }
    }
}

} // namespace TetroShift
