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

bool OrthogonalGrid::UpdateSandPhysics() {
    bool movedAny = false;
    // Scan bottom-to-top so falling grains don't teleport down in a single tick
    for (int y = m_height - 2; y >= -m_bufferHeight; --y) {
        for (int step = 0; step < m_width; ++step) {
            int x = (y % 2 == 0) ? step : (m_width - 1 - step);
            const int idx = CoordToIndex({ x, y });
            if (idx < 0) continue;

            if (m_cells[idx].type == CellType::Sand) {
                GridCoord below{ x, y + 1 };
                if (below.y < m_height && !IsCellOccupied(below)) {
                    // Straight drop downward
                    int belowIdx = CoordToIndex(below);
                    m_cells[belowIdx] = m_cells[idx];
                    m_cells[idx].Clear();
                    movedAny = true;
                } else if (below.y < m_height) {
                    // Try sliding diagonally down-left or down-right
                    bool preferLeft = ((x + y) % 2 == 0);
                    GridCoord diag1 = preferLeft ? GridCoord{ x - 1, y + 1 } : GridCoord{ x + 1, y + 1 };
                    GridCoord side1 = preferLeft ? GridCoord{ x - 1, y } : GridCoord{ x + 1, y };
                    GridCoord diag2 = preferLeft ? GridCoord{ x + 1, y + 1 } : GridCoord{ x - 1, y + 1 };
                    GridCoord side2 = preferLeft ? GridCoord{ x + 1, y } : GridCoord{ x - 1, y };

                    if (diag1.x >= 0 && diag1.x < m_width && diag1.y < m_height && !IsCellOccupied(diag1) && !IsCellOccupied(side1)) {
                        int targetIdx = CoordToIndex(diag1);
                        m_cells[targetIdx] = m_cells[idx];
                        m_cells[idx].Clear();
                        movedAny = true;
                    } else if (diag2.x >= 0 && diag2.x < m_width && diag2.y < m_height && !IsCellOccupied(diag2) && !IsCellOccupied(side2)) {
                        int targetIdx = CoordToIndex(diag2);
                        m_cells[targetIdx] = m_cells[idx];
                        m_cells[idx].Clear();
                        movedAny = true;
                    }
                }
            }
        }
    }
    return movedAny;
}

void OrthogonalGrid::ExplodeArea(const GridCoord& center, int radius, std::vector<GridCoord>& outDestroyed) {
    std::vector<GridCoord> secondaryBombs;

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            GridCoord target{ center.x + dx, center.y + dy };
            if (IsValidCoord(target) && target.y >= 0) {
                int idx = CoordToIndex(target);
                if (idx >= 0 && m_cells[idx].IsSolid()) {
                    outDestroyed.push_back(target);
                    if (m_cells[idx].type == CellType::Bomb && (dx != 0 || dy != 0)) {
                        secondaryBombs.push_back(target);
                    }
                    m_cells[idx].Clear();
                }
            }
        }
    }

    // Cascade chain reactions for adjacent bombs
    for (const auto& sb : secondaryBombs) {
        ExplodeArea(sb, radius, outDestroyed);
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
            } else if (cell.type == CellType::Sand) {
                DrawCircle(static_cast<int>(worldPos.x + cellSize * 0.35f), static_cast<int>(worldPos.y + cellSize * 0.35f), 1.5f, Fade(WHITE, 0.6f));
                DrawCircle(static_cast<int>(worldPos.x + cellSize * 0.65f), static_cast<int>(worldPos.y + cellSize * 0.65f), 1.5f, Fade(DARKGRAY, 0.4f));
                DrawCircle(static_cast<int>(worldPos.x + cellSize * 0.35f), static_cast<int>(worldPos.y + cellSize * 0.70f), 1.2f, Fade(WHITE, 0.4f));
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

void OrthogonalGrid::PushGarbageRow(int holeCol, CellType type, Color color) {
    // Shift rows up by 1 (row 0 is pushed into the void)
    for (int y = 0; y < m_height - 1; ++y) {
        for (int x = 0; x < m_width; ++x) {
            SetCell({ x, y }, GetCell({ x, y + 1 }));
        }
    }

    // Insert new garbage row at the bottom
    for (int x = 0; x < m_width; ++x) {
        if (x == holeCol) {
            SetCell({ x, m_height - 1 }, Cell{});
        } else {
            Cell c;
            c.type = type;
            c.color = color;
            c.isLocked = true;
            c.squishOffset = 0.0f;
            SetCell({ x, m_height - 1 }, c);
        }
    }
}

void OrthogonalGrid::VaporizeTopRows(int count) {
    for (int y = 0; y < count && y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            SetCell({ x, y }, Cell{});
        }
    }
}

void OrthogonalGrid::VaporizeBottomRow() {
    // Clear bottom row
    for (int x = 0; x < m_width; ++x) {
        SetCell({ x, m_height - 1 }, Cell{});
    }

    // Shift all rows down by 1
    for (int y = m_height - 1; y > 0; --y) {
        for (int x = 0; x < m_width; ++x) {
            SetCell({ x, y }, GetCell({ x, y - 1 }));
        }
    }

    // Clear top row
    for (int x = 0; x < m_width; ++x) {
        SetCell({ x, 0 }, Cell{});
    }
}

void OrthogonalGrid::CollapseFloatingCells() {
    for (int x = 0; x < m_width; ++x) {
        std::vector<Cell> solids;
        for (int y = m_height - 1; y >= 0; --y) {
            const Cell& c = GetCell({ x, y });
            if (c.IsSolid()) {
                solids.push_back(c);
            }
        }

        // Place them down from the bottom
        int writeY = m_height - 1;
        for (const auto& c : solids) {
            SetCell({ x, writeY }, c);
            writeY--;
        }
        while (writeY >= 0) {
            SetCell({ x, writeY }, Cell{});
            writeY--;
        }
    }
}

std::vector<std::string> OrthogonalGrid::Serialize() const {
    std::vector<std::string> rows;
    rows.reserve(m_height);
    for (int y = 0; y < m_height; ++y) {
        std::string row;
        row.reserve(m_width);
        for (int x = 0; x < m_width; ++x) {
            const Cell& c = GetCell({ x, y });
            if (!c.IsSolid()) {
                row.push_back('.');
            } else {
                switch (c.type) {
                    case CellType::Bomb:         row.push_back('B'); break;
                    case CellType::Gold:         row.push_back('G'); break;
                    case CellType::Jelly:        row.push_back('E'); break;
                    case CellType::HeavyIron:    row.push_back('H'); break;
                    case CellType::QuantumGhost: row.push_back('Q'); break;
                    case CellType::Glitch:       row.push_back('X'); break;
                    default: {
                        if (c.color.r > 200 && c.color.g < 100 && c.color.b < 100) row.push_back('Z');
                        else if (c.color.r < 100 && c.color.g > 200 && c.color.b < 100) row.push_back('S');
                        else if (c.color.r < 100 && c.color.g > 200 && c.color.b > 200) row.push_back('I');
                        else if (c.color.r < 100 && c.color.g < 100 && c.color.b > 200) row.push_back('J');
                        else if (c.color.r > 200 && c.color.g > 100 && c.color.b < 100) row.push_back('L');
                        else if (c.color.r > 200 && c.color.g > 200 && c.color.b < 100) row.push_back('O');
                        else if (c.color.r > 150 && c.color.g < 100 && c.color.b > 150) row.push_back('T');
                        else row.push_back('S');
                        break;
                    }
                }
            }
        }
        rows.push_back(row);
    }
    return rows;
}

void OrthogonalGrid::Deserialize(const std::vector<std::string>& data) {
    Clear();
    for (int y = 0; y < m_height && y < static_cast<int>(data.size()); ++y) {
        const std::string& row = data[y];
        for (int x = 0; x < m_width && x < static_cast<int>(row.size()); ++x) {
            char ch = row[x];
            if (ch == '.') continue;
            Cell c;
            c.type = CellType::Solid;
            switch (ch) {
                case 'B': c.type = CellType::Bomb; c.color = Colors::PieceBomb; break;
                case 'G': c.type = CellType::Gold; c.color = Colors::PieceGold; break;
                case 'E': c.type = CellType::Jelly; c.color = Colors::PieceJelly; break;
                case 'H': c.type = CellType::HeavyIron; c.color = Colors::PieceIron; break;
                case 'Q': c.type = CellType::QuantumGhost; c.color = Colors::PieceI; break;
                case 'X': c.type = CellType::Glitch; c.color = Colors::PieceZ; break;
                case 'I': c.color = Colors::PieceI; break;
                case 'J': c.color = Colors::PieceJ; break;
                case 'L': c.color = Colors::PieceL; break;
                case 'O': c.color = Colors::PieceO; break;
                case 'S': c.color = Colors::PieceS; break;
                case 'T': c.color = Colors::PieceT; break;
                case 'Z': c.color = Colors::PieceZ; break;
                default:  c.color = Colors::PieceI; break;
            }
            SetCell({ x, y }, c);
        }
    }
}

} // namespace TetroShift
