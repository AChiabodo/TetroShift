#include "ActivePiece.hpp"
#include "grid/IGrid.hpp"
#include "core/EventBus.hpp"
#include "core/Constants.hpp"
#include <algorithm>

namespace TetroShift {

void ActivePiece::Spawn(TetrominoType type, GridCoord startPos, CellType minoType, float elasticity) {
    m_type = type;
    m_position = startPos;
    m_rotation = 0;
    m_minoType = minoType;
    m_fallTimer = 0.0f;
    m_lockTimer = 0.0f;
    m_lockMoveCount = 0;
    m_isGrounded = false;
    m_isLocked = false;
    m_ghostPhaseTimer = 0.0f;
    m_lastActionWasRotate = false;
    m_lastKickIndex = 0;

    m_softBody.SetElasticity(elasticity);

    // Initial dummy positions for soft body mesh (will be aligned on first update/render)
    std::vector<Vector2> initialPos(4, Vector2{ 0.0f, 0.0f });
    m_softBody.Initialize(initialPos);
}

std::array<GridCoord, 4> ActivePiece::GetMinoGridCoordsAt(GridCoord pos, int rot) const noexcept {
    const auto localOffsets = TetrominoDefinition::GetMinoCoords(m_type, rot);
    std::array<GridCoord, 4> globalCoords{};
    for (size_t i = 0; i < 4; ++i) {
        globalCoords[i] = pos + localOffsets[i];
    }
    return globalCoords;
}

std::array<GridCoord, 4> ActivePiece::GetMinoGridCoords() const noexcept {
    return GetMinoGridCoordsAt(m_position, m_rotation);
}

bool ActivePiece::CheckCollision(GridCoord pos, int rot, const IGrid& grid) const noexcept {
    const auto coords = GetMinoGridCoordsAt(pos, rot);
    bool isRadial = (grid.GetGeometryType() == GridGeometry::Radial);

    for (const auto& rawC : coords) {
        GridCoord c = rawC;
        if (isRadial) {
            c.x = (c.x % grid.GetWidth() + grid.GetWidth()) % grid.GetWidth();
        }

        // Out of horizontal bounds for non-radial grids
        if (!isRadial && (c.x < 0 || c.x >= grid.GetWidth())) {
            return true;
        }

        // Below floor or hitting singularity core
        if (c.y >= grid.GetHeight()) {
            return true;
        }

        // Occupied cell check (bypassed if quantum ghost phase active)
        if (m_ghostPhaseTimer <= 0.0f) {
            if (c.y >= 0 && grid.IsCellOccupied(c)) {
                return true;
            }
        }
    }
    return false;
}

bool ActivePiece::TryMove(GridCoord delta, const IGrid& grid, EventBus* eventBus) {
    if (m_isLocked) return false;

    GridCoord newPos = m_position + delta;
    if (!CheckCollision(newPos, m_rotation, grid)) {
        m_position = newPos;
        m_lastActionWasRotate = false;

        // Apply physical wobble impulse
        const float impulseMag = (delta.x != 0) ? WOBBLE_IMPULSE_MOVE : 4.0f;
        m_softBody.ApplyImpulse({ static_cast<float>(delta.x) * impulseMag, static_cast<float>(delta.y) * impulseMag });

        // Reset lock timer if piece was touching ground and hasn't exceeded move cap
        if (m_isGrounded && m_lockMoveCount < MAX_LOCK_MOVES) {
            m_lockTimer = 0.0f;
            m_lockMoveCount++;
        }

        if (eventBus) {
            eventBus->Publish(EventPieceMove{ delta.x, delta.y, true });
        }
        return true;
    }

    if (eventBus) {
        eventBus->Publish(EventPieceMove{ delta.x, delta.y, false });
    }
    return false;
}

bool ActivePiece::TryRotate(int direction, const IGrid& grid, EventBus* eventBus) {
    if (m_isLocked) return false;

    // O piece does not rotate
    if (m_type == TetrominoType::O) {
        m_softBody.ApplyRotationTorque(static_cast<float>(direction) * 20.0f);
        m_lastActionWasRotate = false;
        return true;
    }

    const int targetRot = (m_rotation + direction + 4) % 4;
    const auto kicks = TetrominoDefinition::GetWallKicks(m_type, m_rotation, targetRot);

    for (size_t i = 0; i < kicks.size(); ++i) {
        GridCoord testPos = m_position + kicks[i];
        if (!CheckCollision(testPos, targetRot, grid)) {
            m_position = testPos;
            m_rotation = targetRot;
            m_lastActionWasRotate = true;
            m_lastKickIndex = i;

            // Apply rotational spring torque
            m_softBody.ApplyRotationTorque(static_cast<float>(direction) * WOBBLE_IMPULSE_ROTATE);

            if (m_isGrounded && m_lockMoveCount < MAX_LOCK_MOVES) {
                m_lockTimer = 0.0f;
                m_lockMoveCount++;
            }

            if (eventBus) {
                eventBus->Publish(EventPieceRotate{ m_rotation, i > 0 });
            }
            return true;
        }
    }

    return false;
}

TSpinType ActivePiece::CheckTSpin(const IGrid& grid) const noexcept {
    if (m_type != TetrominoType::T || !m_lastActionWasRotate) {
        return TSpinType::None;
    }

    // In 3x3 T-piece grid, center is (x+1, y+1)
    GridCoord center = m_position + GridCoord{ 1, 1 };

    GridCoord nw = center + GridCoord{ -1, -1 };
    GridCoord ne = center + GridCoord{ +1, -1 };
    GridCoord se = center + GridCoord{ +1, +1 };
    GridCoord sw = center + GridCoord{ -1, +1 };

    auto isOccupied = [&grid](const GridCoord& c) -> bool {
        if (c.x < 0 || c.x >= grid.GetWidth() || c.y >= grid.GetHeight()) {
            return true; // Walls and floor count as occupied
        }
        return (c.y >= 0) ? grid.IsCellOccupied(c) : false;
    };

    bool nwOcc = isOccupied(nw);
    bool neOcc = isOccupied(ne);
    bool seOcc = isOccupied(se);
    bool swOcc = isOccupied(sw);

    int totalOccupied = (nwOcc ? 1 : 0) + (neOcc ? 1 : 0) + (seOcc ? 1 : 0) + (swOcc ? 1 : 0);
    if (totalOccupied < 3) {
        return TSpinType::None;
    }

    // Identify front corners based on rotation
    // rot 0 (North): front = NW, NE; back = SW, SE
    // rot 1 (East):  front = NE, SE; back = NW, SW
    // rot 2 (South): front = SW, SE; back = NW, NE
    // rot 3 (West):  front = NW, SW; back = NE, SE
    int frontOccupied = 0;
    switch (m_rotation) {
        case 0: frontOccupied = (nwOcc ? 1 : 0) + (neOcc ? 1 : 0); break;
        case 1: frontOccupied = (neOcc ? 1 : 0) + (seOcc ? 1 : 0); break;
        case 2: frontOccupied = (swOcc ? 1 : 0) + (seOcc ? 1 : 0); break;
        case 3: frontOccupied = (nwOcc ? 1 : 0) + (swOcc ? 1 : 0); break;
        default: frontOccupied = 0; break;
    }

    if (frontOccupied == 2) {
        return TSpinType::Standard;
    }

    // Front occupied == 1, back occupied == 2
    // If kick test 5 (index 4 in 0-based indexing) was used -> Standard (T-Spin Triple exception)
    if (m_lastKickIndex == 4) {
        return TSpinType::Standard;
    }

    return TSpinType::Mini;
}

GridCoord ActivePiece::GetGhostPosition(const IGrid& grid) const noexcept {
    GridCoord ghostPos = m_position;
    while (!CheckCollision(ghostPos + GridCoord{ 0, 1 }, m_rotation, grid)) {
        ghostPos.y += 1;
    }
    return ghostPos;
}

int ActivePiece::HardDrop(const IGrid& grid, EventBus* eventBus) {
    if (m_isLocked) return 0;

    int linesDropped = 0;
    while (!CheckCollision(m_position + GridCoord{ 0, 1 }, m_rotation, grid)) {
        m_position.y += 1;
        linesDropped++;
    }

    if (linesDropped > 0) {
        m_lastActionWasRotate = false;
    }

    m_isGrounded = true;
    m_isLocked = true;

    // Trigger powerful impact squish
    m_softBody.TriggerSquish({ 0.0f, static_cast<float>(linesDropped) * WOBBLE_IMPULSE_DROP });

    if (eventBus) {
        eventBus->Publish(EventPieceHardDrop{ linesDropped, m_position.y });
    }

    return linesDropped;
}

std::vector<Vector2> ActivePiece::GetTargetWorldPositions(const IGrid& grid, Vector2 gridOrigin, float cellSize) const {
    const auto coords = GetMinoGridCoords();
    std::vector<Vector2> targets;
    targets.reserve(4);
    for (const auto& c : coords) {
        targets.push_back(grid.CoordToWorld(c, gridOrigin, cellSize));
    }
    return targets;
}

void ActivePiece::Update(float dt, float fallInterval, const IGrid& grid, EventBus* eventBus) {
    if (m_isLocked) return;

    if (m_ghostPhaseTimer > 0.0f) {
        m_ghostPhaseTimer = std::max(0.0f, m_ghostPhaseTimer - dt);
    }

    // Check if resting on floor / obstacle
    m_isGrounded = CheckCollision(m_position + GridCoord{ 0, 1 }, m_rotation, grid);

    if (m_isGrounded) {
        m_lockTimer += dt;
        if (m_lockTimer >= BASE_LOCK_DELAY) {
            m_isLocked = true;
            if (eventBus) {
                const auto coords = GetMinoGridCoords();
                std::vector<GridCoord> lockedCells(coords.begin(), coords.end());
                eventBus->Publish(EventPieceLock{ m_type, lockedCells });
            }
        }
    } else {
        m_lockTimer = 0.0f;
        m_fallTimer += dt;
        if (m_fallTimer >= fallInterval) {
            m_fallTimer = 0.0f;
            TryMove(GridCoord{ 0, 1 }, grid, eventBus);
        }
    }
}

void ActivePiece::Render(const IGrid& grid, Vector2 gridOrigin, float cellSize, bool showGhost, bool showDebug) const {
    GridGeometry geom = grid.GetGeometryType();

    // 1. Draw Ghost Piece
    if (showGhost && !m_isLocked) {
        const GridCoord ghostPos = GetGhostPosition(grid);
        if (ghostPos.y != m_position.y) {
            const auto ghostCoords = GetMinoGridCoordsAt(ghostPos, m_rotation);
            Color ghostColor = GetTetrominoColor(m_type);
            ghostColor.a = 75; // Subtle transparent

            for (const auto& rawC : ghostCoords) {
                GridCoord c = rawC;
                if (geom == GridGeometry::Radial) {
                    c.x = (c.x % grid.GetWidth() + grid.GetWidth()) % grid.GetWidth();
                }

                if (c.y >= 0) {
                    Vector2 worldPos = grid.CoordToWorld(c, gridOrigin, cellSize);

                    if (geom == GridGeometry::Hexagonal) {
                        float radius = 19.5f;
                        Vector2 center = { worldPos.x + cellSize * 0.5f, worldPos.y + cellSize * 0.5f };
                        DrawPoly(center, 6, radius - 2.0f, 30.0f, Fade(ghostColor, 0.25f));
                        DrawPolyLinesEx(center, 6, radius - 1.5f, 30.0f, 1.5f, ghostColor);
                    } else if (geom == GridGeometry::Radial) {
                        float pillRadius = 10.0f;
                        DrawCircleV(worldPos, pillRadius, Fade(ghostColor, 0.25f));
                        DrawCircleLines(static_cast<int>(worldPos.x), static_cast<int>(worldPos.y), pillRadius, ghostColor);
                    } else {
                        Rectangle r = { worldPos.x + 2.0f, worldPos.y + 2.0f, cellSize - 4.0f, cellSize - 4.0f };
                        DrawRectangleRounded(r, 0.2f, 4, ghostColor);
                        DrawRectangleLinesEx(r, 1.5f, Fade(ghostColor, 0.8f));
                    }
                }
            }
        }
    }

    // 2. Draw Active Piece using SoftBodyMesh
    const auto targetWorldPos = GetTargetWorldPositions(grid, gridOrigin, cellSize);
    // Update soft body position towards targets
    const_cast<ActivePiece*>(this)->m_softBody.Update(GetFrameTime(), targetWorldPos);

    Color pieceColor = GetTetrominoColor(m_type);
    if (m_minoType == CellType::Gold) pieceColor = Colors::PieceGold;
    else if (m_minoType == CellType::Bomb) pieceColor = Colors::PieceBomb;
    else if (m_minoType == CellType::Jelly) pieceColor = Colors::PieceJelly;

    if (m_ghostPhaseTimer > 0.0f) {
        pieceColor.a = 160; // Hologram glow
    }

    m_softBody.Render(targetWorldPos, pieceColor, cellSize, geom, showDebug);
}

} // namespace TetroShift
