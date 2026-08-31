#include "RadialGrid.hpp"
#include "core/Constants.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace TetroShift {

RadialGrid::RadialGrid(int numSectors, int numRings, int bufferRings)
    : m_numSectors(numSectors), m_numRings(numRings), m_bufferRings(bufferRings) {
    Initialize(numSectors, numRings, bufferRings);
}

void RadialGrid::Initialize(int width, int height, int bufferHeight) {
    m_numSectors = width;
    m_numRings = height;
    m_bufferRings = bufferHeight;
    m_cells.resize(m_numSectors * (m_numRings + m_bufferRings));
    Clear();
}

void RadialGrid::Clear() {
    for (auto& cell : m_cells) {
        cell.Clear();
    }
}

int RadialGrid::NormalizeSector(int x) const noexcept {
    return (x % m_numSectors + m_numSectors) % m_numSectors;
}

int RadialGrid::CoordToIndex(const GridCoord& coord) const noexcept {
    const int normX = NormalizeSector(coord.x);
    const int effectiveY = coord.y + m_bufferRings;
    if (effectiveY < 0 || effectiveY >= (m_numRings + m_bufferRings)) {
        return -1;
    }
    return effectiveY * m_numSectors + normX;
}

bool RadialGrid::IsValidCoord(const GridCoord& coord) const noexcept {
    const int effectiveY = coord.y + m_bufferRings;
    return effectiveY >= 0 && effectiveY < (m_numRings + m_bufferRings);
}

bool RadialGrid::IsCellOccupied(const GridCoord& coord) const noexcept {
    // In radial grid, hitting ring >= m_numRings hits the central core singularity!
    if (coord.y >= m_numRings) {
        return true;
    }
    const int idx = CoordToIndex(coord);
    if (idx < 0) {
        return false;
    }
    return m_cells[idx].IsSolid();
}

const Cell& RadialGrid::GetCell(const GridCoord& coord) const {
    const int idx = CoordToIndex(coord);
    if (idx < 0) {
        return m_emptyCell;
    }
    return m_cells[idx];
}

void RadialGrid::SetCell(const GridCoord& coord, const Cell& cell) {
    const int idx = CoordToIndex(coord);
    if (idx >= 0 && idx < static_cast<int>(m_cells.size())) {
        m_cells[idx] = cell;
    }
}

void RadialGrid::Update(float dt) {
    m_pulseTimer += dt;

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

void RadialGrid::ExplodeArea(const GridCoord& center, int radius, std::vector<GridCoord>& outDestroyed) {
    std::vector<GridCoord> secondaryBombs;

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            GridCoord target{ center.x + dx, center.y + dy };
            if (IsValidCoord(target) && target.y >= 0) {
                int idx = CoordToIndex(target);
                if (idx >= 0 && m_cells[idx].IsSolid()) {
                    outDestroyed.push_back({ NormalizeSector(target.x), target.y });
                    if (m_cells[idx].type == CellType::Bomb && (dx != 0 || dy != 0)) {
                        secondaryBombs.push_back({ NormalizeSector(target.x), target.y });
                    }
                    m_cells[idx].Clear();
                }
            }
        }
    }

    for (const auto& sb : secondaryBombs) {
        ExplodeArea(sb, radius, outDestroyed);
    }
}

LineClearResult RadialGrid::CheckAndClearLines() {
    LineClearResult result;
    std::vector<int> fullRings;

    // 1. Check Full Concentric Rings (360° Ring Clears)
    for (int ring = m_numRings - 1; ring >= 0; --ring) {
        bool isFull = true;
        for (int sec = 0; sec < m_numSectors; ++sec) {
            const int idx = CoordToIndex({ sec, ring });
            if (idx < 0 || !m_cells[idx].IsSolid()) {
                isFull = false;
                break;
            }
        }
        if (isFull) {
            fullRings.push_back(ring);
        }
    }

    if (fullRings.empty()) {
        return result;
    }

    result.linesCount = static_cast<int>(fullRings.size());
    result.rowsCleared = fullRings;
    result.isTetris = (result.linesCount >= 4);

    for (int ring : fullRings) {
        for (int sec = 0; sec < m_numSectors; ++sec) {
            GridCoord c{ sec, ring };
            result.clearedCells.push_back(c);

            const int idx = CoordToIndex(c);
            if (idx >= 0) {
                if (m_cells[idx].type == CellType::Gold) {
                    result.coinsGenerated += 6;
                } else if (m_cells[idx].type == CellType::Bomb) {
                    result.bombExplosions.push_back(c);
                }
            }
        }
    }

    // Process bomb explosions
    for (const auto& bombCoord : result.bombExplosions) {
        std::vector<GridCoord> destroyedByBlast;
        ExplodeArea(bombCoord, 2, destroyedByBlast);
        for (const auto& d : destroyedByBlast) {
            result.clearedCells.push_back(d);
        }
    }

    // Centripetal inward collapse (outer rings move 1 step inward towards core)
    for (int clearedRing : fullRings) {
        for (int ring = clearedRing; ring > -m_bufferRings; --ring) {
            for (int sec = 0; sec < m_numSectors; ++sec) {
                int currIdx = CoordToIndex({ sec, ring });
                int outerIdx = CoordToIndex({ sec, ring - 1 });
                if (currIdx >= 0) {
                    if (outerIdx >= 0) {
                        m_cells[currIdx] = m_cells[outerIdx];
                    } else {
                        m_cells[currIdx].Clear();
                    }
                }
            }
        }
        for (int sec = 0; sec < m_numSectors; ++sec) {
            int topIdx = CoordToIndex({ sec, -m_bufferRings });
            if (topIdx >= 0) {
                m_cells[topIdx].Clear();
            }
        }
    }

    return result;
}

Vector2 RadialGrid::CoordToWorld(const GridCoord& coord, Vector2 gridOrigin, float /*cellSize*/) const noexcept {
    const Vector2 center = { gridOrigin.x + 190.0f, gridOrigin.y + 240.0f };
    const float normSec = static_cast<float>(NormalizeSector(coord.x));
    const float angle = (normSec + 0.5f) * (2.0f * std::numbers::pi_v<float> / static_cast<float>(m_numSectors)) - (std::numbers::pi_v<float> * 0.5f);

    const float depthRatio = static_cast<float>(coord.y) / static_cast<float>(std::max(1, m_numRings - 1));
    const float r = m_outerRadius - depthRatio * (m_outerRadius - m_innerRadius);

    return {
        center.x + std::cos(angle) * r,
        center.y + std::sin(angle) * r
    };
}

void RadialGrid::Render(Vector2 gridOrigin, float cellSize, bool showDebugHitbox) const {
    const Vector2 center = { gridOrigin.x + 190.0f, gridOrigin.y + 240.0f };

    // 1. Draw Central Singularity Core
    float pulse = (std::sin(m_pulseTimer * 3.5f) + 1.0f) * 0.5f;
    DrawCircleGradient(
        static_cast<int>(center.x),
        static_cast<int>(center.y),
        m_innerRadius + 8.0f,
        Fade(Colors::PieceT, 0.45f + pulse * 0.35f),
        Fade(Colors::BgDark, 0.0f)
    );
    DrawCircle(static_cast<int>(center.x), static_cast<int>(center.y), m_innerRadius - 4.0f, Colors::BgPanel);
    DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), m_innerRadius - 4.0f, Colors::PieceI);
    DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), m_innerRadius + 2.0f, Fade(Colors::PieceT, 0.5f));

    // Core icon / text
    DrawText("CORE", static_cast<int>(center.x - 14.0f), static_cast<int>(center.y - 6.0f), 10, Colors::PieceI);

    // 2. Concentric Orbit Rings
    const float ringStep = (m_outerRadius - m_innerRadius) / static_cast<float>(m_numRings);
    for (int r = 0; r <= m_numRings; ++r) {
        float rad = m_innerRadius + static_cast<float>(r) * ringStep;
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), rad, (r == m_numRings) ? Colors::BgPanelBorder : Colors::GridLine);
    }

    // 3. Radial Spoke Dividers (every sector)
    for (int s = 0; s < m_numSectors; ++s) {
        float angle = static_cast<float>(s) * (2.0f * std::numbers::pi_v<float> / static_cast<float>(m_numSectors)) - (std::numbers::pi_v<float> * 0.5f);
        Vector2 innerPt = { center.x + std::cos(angle) * m_innerRadius, center.y + std::sin(angle) * m_innerRadius };
        Vector2 outerPt = { center.x + std::cos(angle) * (m_outerRadius + 8.0f), center.y + std::sin(angle) * (m_outerRadius + 8.0f) };
        DrawLineV(innerPt, outerPt, Colors::GridLine);
    }

    // 4. Locked Solid Cells (Arc Quads)
    for (int ring = 0; ring < m_numRings; ++ring) {
        for (int sec = 0; sec < m_numSectors; ++sec) {
            const int idx = CoordToIndex({ sec, ring });
            if (idx < 0) continue;

            const Cell& cell = m_cells[idx];
            if (!cell.IsSolid()) continue;

            Vector2 cellCenter = CoordToWorld({ sec, ring }, gridOrigin, cellSize);

            Color drawColor = cell.color;
            if (cell.flashTimer > 0.0f) {
                drawColor = ColorAlphaBlend(drawColor, WHITE, Fade(WHITE, cell.flashTimer));
            }

            // Draw rounded orbital mino pill
            const float pillRadius = ringStep * 0.44f;
            DrawCircleV(cellCenter, pillRadius, drawColor);
            DrawCircleLines(static_cast<int>(cellCenter.x), static_cast<int>(cellCenter.y), pillRadius, ColorAlphaBlend(drawColor, WHITE, Fade(WHITE, 0.4f)));
            DrawCircle(static_cast<int>(cellCenter.x), static_cast<int>(cellCenter.y), pillRadius * 0.4f, Fade(BLACK, 0.35f));

            // Icons
            if (cell.type == CellType::Bomb) {
                DrawCircle(static_cast<int>(cellCenter.x), static_cast<int>(cellCenter.y), pillRadius * 0.45f, RED);
                DrawCircle(static_cast<int>(cellCenter.x), static_cast<int>(cellCenter.y), pillRadius * 0.22f, YELLOW);
            } else if (cell.type == CellType::Gold) {
                DrawCircleLines(static_cast<int>(cellCenter.x), static_cast<int>(cellCenter.y), pillRadius * 0.5f, GOLD);
            } else if (cell.type == CellType::Jelly) {
                DrawCircle(static_cast<int>(cellCenter.x), static_cast<int>(cellCenter.y), pillRadius * 0.35f, Fade(WHITE, 0.6f));
            } else if (cell.type == CellType::Sand) {
                DrawCircle(static_cast<int>(cellCenter.x - 2.0f), static_cast<int>(cellCenter.y - 2.0f), 1.5f, Fade(WHITE, 0.6f));
                DrawCircle(static_cast<int>(cellCenter.x + 2.0f), static_cast<int>(cellCenter.y + 2.0f), 1.5f, Fade(DARKGRAY, 0.4f));
            }

            if (showDebugHitbox) {
                DrawCircleLines(static_cast<int>(cellCenter.x), static_cast<int>(cellCenter.y), pillRadius, MAGENTA);
            }
        }
    }
}

void RadialGrid::PushGarbageRow(int holeCol, CellType type, Color color) {
    // Shifts all rings outward by 1
    for (int ring = -m_bufferRings; ring < m_numRings - 1; ++ring) {
        for (int sec = 0; sec < m_numSectors; ++sec) {
            int currIdx = CoordToIndex({ sec, ring });
            int nextIdx = CoordToIndex({ sec, ring + 1 });
            if (currIdx >= 0 && nextIdx >= 0) {
                m_cells[currIdx] = m_cells[nextIdx];
            }
        }
    }

    int normHole = NormalizeSector(holeCol);
    for (int sec = 0; sec < m_numSectors; ++sec) {
        int bIdx = CoordToIndex({ sec, m_numRings - 1 });
        if (bIdx >= 0) {
            if (sec == normHole) {
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

void RadialGrid::VaporizeTopRows(int count) {
    for (int ring = 0; ring < count && ring < m_numRings; ++ring) {
        for (int sec = 0; sec < m_numSectors; ++sec) {
            int idx = CoordToIndex({ sec, ring });
            if (idx >= 0) {
                m_cells[idx].Clear();
            }
        }
    }
}

void RadialGrid::VaporizeBottomRow() {
    for (int sec = 0; sec < m_numSectors; ++sec) {
        int idx = CoordToIndex({ sec, m_numRings - 1 });
        if (idx >= 0) {
            m_cells[idx].Clear();
        }
    }
}

void RadialGrid::CollapseFloatingCells() {
    for (int ring = m_numRings - 2; ring >= 0; --ring) {
        for (int sec = 0; sec < m_numSectors; ++sec) {
            int idx = CoordToIndex({ sec, ring });
            if (idx >= 0 && m_cells[idx].IsSolid()) {
                GridCoord below{ sec, ring + 1 };
                if (!IsCellOccupied(below)) {
                    int belowIdx = CoordToIndex(below);
                    if (belowIdx >= 0) {
                        m_cells[belowIdx] = m_cells[idx];
                        m_cells[idx].Clear();
                    }
                }
            }
        }
    }
}

std::vector<std::string> RadialGrid::Serialize() const {
    std::vector<std::string> rows;
    rows.reserve(m_numRings);
    for (int ring = 0; ring < m_numRings; ++ring) {
        std::string rowStr;
        for (int sec = 0; sec < m_numSectors; ++sec) {
            const int idx = CoordToIndex({ sec, ring });
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

void RadialGrid::Deserialize(const std::vector<std::string>& data) {
    Clear();
    for (int ring = 0; ring < static_cast<int>(data.size()) && ring < m_numRings; ++ring) {
        const std::string& row = data[ring];
        for (int sec = 0; sec < static_cast<int>(row.size()) && sec < m_numSectors; ++sec) {
            if (row[sec] != '.') {
                int t = row[sec] - '0';
                Cell c;
                c.type = static_cast<CellType>(t);
                c.color = (c.type == CellType::Bomb) ? Colors::PieceBomb :
                          (c.type == CellType::Gold) ? Colors::PieceGold :
                          (c.type == CellType::Jelly) ? Colors::PieceJelly :
                          (c.type == CellType::Sand) ? Colors::PieceSand : Colors::PieceI;
                c.isLocked = true;
                SetCell({ sec, ring }, c);
            }
        }
    }
}

} // namespace TetroShift
