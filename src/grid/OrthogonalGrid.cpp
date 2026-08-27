#include "OrthogonalGrid.hpp"
#include "core/Constants.hpp"
#include <algorithm>
#include <cmath>

namespace TetroShift {

OrthogonalGrid::OrthogonalGrid(int width, int height, int bufferHeight)
    : m_width(width), m_height(height), m_bufferHeight(bufferHeight) {
    Initialize(width, height, bufferHeight);
}

void OrthogonalGrid::Initialize(int width, int height, int bufferHeight) {
    m_width = width;
    m_height = height;
    m_bufferHeight = bufferHeight;
    m_cells.resize(m_width * (m_height + m_bufferHeight));
    Clear();
}

void OrthogonalGrid::Clear() {
    for (auto& cell : m_cells) {
        cell.Clear();
    }
}

int OrthogonalGrid::CoordToIndex(const GridCoord& coord) const noexcept {
    const int effectiveY = coord.y + m_bufferHeight;
    if (coord.x < 0 || coord.x >= m_width || effectiveY < 0 || effectiveY >= (m_height + m_bufferHeight)) {
        return -1;
    }
    return effectiveY * m_width + coord.x;
}

bool OrthogonalGrid::IsValidCoord(const GridCoord& coord) const noexcept {
    const int effectiveY = coord.y + m_bufferHeight;
    return coord.x >= 0 && coord.x < m_width && effectiveY >= 0 && effectiveY < (m_height + m_bufferHeight);
}

bool OrthogonalGrid::IsCellOccupied(const GridCoord& coord) const noexcept {
    const int idx = CoordToIndex(coord);
    if (idx < 0) {
        // Out of bounds: treat bottom and sides as occupied, top buffer as open if x is in bounds
        if (coord.x < 0 || coord.x >= m_width || coord.y >= m_height) {
            return true;
        }
        return false;
    }
    return m_cells[idx].IsSolid();
}

const Cell& OrthogonalGrid::GetCell(const GridCoord& coord) const {
    const int idx = CoordToIndex(coord);
    if (idx < 0) {
        return m_emptyCell;
    }
    return m_cells[idx];
}

void OrthogonalGrid::SetCell(const GridCoord& coord, const Cell& cell) {
    const int idx = CoordToIndex(coord);
    if (idx >= 0 && idx < static_cast<int>(m_cells.size())) {
        m_cells[idx] = cell;
    }
}

void OrthogonalGrid::Update(float dt) {
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

LineClearResult OrthogonalGrid::CheckAndClearLines() {
    LineClearResult result;
    std::vector<int> fullRows;

    // Scan from bottom row to top visible row
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

    // Collect cells and handle special modifiers (Bombs, Gold)
    for (int row : fullRows) {
        for (int x = 0; x < m_width; ++x) {
            GridCoord coord{ x, row };
            const Cell& cell = GetCell(coord);
            result.clearedCells.push_back(coord);

            if (cell.type == CellType::Gold) {
                result.coinsGenerated += 5;
            } else if (cell.type == CellType::Bomb) {
                // Trigger 3x3 explosion
                ExplodeArea(coord, 1, result.bombExplosions);
            }
        }
    }

    // Clear and shift rows
    // Since fullRows is sorted descending (e.g. 19, 18), we can copy downward
    int writeY = m_height - 1;
    for (int readY = m_height - 1; readY >= -m_bufferHeight; --readY) {
        // If readY is in fullRows, skip copying it
        if (std::find(fullRows.begin(), fullRows.end(), readY) != fullRows.end()) {
            continue;
        }

        if (writeY != readY) {
            for (int x = 0; x < m_width; ++x) {
                const int srcIdx = CoordToIndex({ x, readY });
                const int dstIdx = CoordToIndex({ x, writeY });
                if (srcIdx >= 0 && dstIdx >= 0) {
                    m_cells[dstIdx] = m_cells[srcIdx];
                }
            }
        }
        --writeY;
    }

    // Clear remaining top rows that were vacated
    for (int y = writeY; y >= -m_bufferHeight; --y) {
        for (int x = 0; x < m_width; ++x) {
            const int idx = CoordToIndex({ x, y });
            if (idx >= 0) {
                m_cells[idx].Clear();
            }
        }
    }

    return result;
}

void OrthogonalGrid::ExplodeArea(const GridCoord& center, int radius, std::vector<GridCoord>& outDestroyed) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            GridCoord target{ center.x + dx, center.y + dy };
            if (IsValidCoord(target) && target.y >= 0) {
                int idx = CoordToIndex(target);
                if (idx >= 0 && m_cells[idx].IsSolid()) {
                    outDestroyed.push_back(target);
                    m_cells[idx].Clear();
                }
            }
        }
    }
}

void OrthogonalGrid::ApplyHorizontalMagneticPull(bool toRight) {
    // Shifts isolated orphan blocks horizontally on each row
    for (int y = 0; y < m_height; ++y) {
        std::vector<Cell> rowCells(m_width);
        std::vector<Cell> nonEmpties;

        for (int x = 0; x < m_width; ++x) {
            rowCells[x] = GetCell({ x, y });
            if (rowCells[x].IsSolid()) {
                nonEmpties.push_back(rowCells[x]);
            }
        }

        if (nonEmpties.empty() || static_cast<int>(nonEmpties.size()) == m_width) {
            continue;
        }

        // Clear row
        for (int x = 0; x < m_width; ++x) {
            SetCell({ x, y }, m_emptyCell);
        }

        // Place to left or right
        if (toRight) {
            int startX = m_width - static_cast<int>(nonEmpties.size());
            for (size_t i = 0; i < nonEmpties.size(); ++i) {
                SetCell({ startX + static_cast<int>(i), y }, nonEmpties[i]);
            }
        } else {
            for (size_t i = 0; i < nonEmpties.size(); ++i) {
                SetCell({ static_cast<int>(i), y }, nonEmpties[i]);
            }
        }
    }
}

Vector2 OrthogonalGrid::CoordToWorld(const GridCoord& coord, Vector2 gridOrigin, float cellSize) const noexcept {
    return {
        gridOrigin.x + static_cast<float>(coord.x) * cellSize,
        gridOrigin.y + static_cast<float>(coord.y) * cellSize
    };
}

void OrthogonalGrid::Render(Vector2 gridOrigin, float cellSize, bool showDebugHitbox) const {
    const float gridPxWidth = static_cast<float>(m_width) * cellSize;
    const float gridPxHeight = static_cast<float>(m_height) * cellSize;

    // Draw Grid Background
    DrawRectangleRec({ gridOrigin.x, gridOrigin.y, gridPxWidth, gridPxHeight }, Colors::GridBg);

    // Draw Sub-grid Grid Lines (faint neon)
    for (int x = 0; x <= m_width; ++x) {
        const float px = gridOrigin.x + static_cast<float>(x) * cellSize;
        DrawLineV({ px, gridOrigin.y }, { px, gridOrigin.y + gridPxHeight }, Colors::GridLine);
    }
    for (int y = 0; y <= m_height; ++y) {
        const float py = gridOrigin.y + static_cast<float>(y) * cellSize;
        DrawLineV({ gridOrigin.x, py }, { gridOrigin.x + gridPxWidth, py }, Colors::GridLine);
    }

    // Draw Locked Solid Cells
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const int idx = CoordToIndex({ x, y });
            if (idx < 0) continue;

            const Cell& cell = m_cells[idx];
            if (!cell.IsSolid()) continue;

            Vector2 worldPos = CoordToWorld({ x, y }, gridOrigin, cellSize);
            worldPos.y += cell.squishOffset;

            const Rectangle cellRect = { worldPos.x + 1.0f, worldPos.y + 1.0f, cellSize - 2.0f, cellSize - 2.0f };

            // Cell Main Body
            Color drawColor = cell.color;
            if (cell.flashTimer > 0.0f) {
                // Bright white flash
                drawColor = ColorAlphaBlend(drawColor, WHITE, Fade(WHITE, cell.flashTimer));
            }

            DrawRectangleRounded(cellRect, 0.2f, 4, drawColor);

            // Subtle 3D Highlight & Shadow on Minos
            DrawRectangleRec({ cellRect.x + 2.0f, cellRect.y + 2.0f, cellRect.width - 4.0f, 3.0f }, Fade(WHITE, 0.35f));
            DrawRectangleRec({ cellRect.x + 2.0f, cellRect.y + cellRect.height - 4.0f, cellRect.width - 4.0f, 2.0f }, Fade(BLACK, 0.4f));

            // Special cell icons/borders
            if (cell.type == CellType::Bomb) {
                DrawCircle(static_cast<int>(worldPos.x + cellSize * 0.5f), static_cast<int>(worldPos.y + cellSize * 0.5f), cellSize * 0.25f, RED);
                DrawCircle(static_cast<int>(worldPos.x + cellSize * 0.5f), static_cast<int>(worldPos.y + cellSize * 0.5f), cellSize * 0.12f, YELLOW);
            } else if (cell.type == CellType::Gold) {
                DrawCircleLines(static_cast<int>(worldPos.x + cellSize * 0.5f), static_cast<int>(worldPos.y + cellSize * 0.5f), cellSize * 0.3f, GOLD);
            } else if (cell.type == CellType::Jelly) {
                DrawCircle(static_cast<int>(worldPos.x + cellSize * 0.5f), static_cast<int>(worldPos.y + cellSize * 0.5f), cellSize * 0.2f, Fade(WHITE, 0.5f));
            }

            // Debug hitboxes
            if (showDebugHitbox) {
                DrawRectangleLinesEx(cellRect, 1.0f, GREEN);
            }
        }
    }

    // Outer Neon Border
    DrawRectangleLinesEx({ gridOrigin.x - 2.0f, gridOrigin.y - 2.0f, gridPxWidth + 4.0f, gridPxHeight + 4.0f }, 2.0f, Colors::BgPanelBorder);
}

} // namespace TetroShift
