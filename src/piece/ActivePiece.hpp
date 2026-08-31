#pragma once
#include "TetrominoType.hpp"
#include "TetrominoDefinition.hpp"
#include "grid/GridCoord.hpp"
#include "grid/Cell.hpp"
#include "physics/SoftBodyMesh.hpp"
#include <array>
#include <vector>
#include <raylib.h>

namespace TetroShift {

class IGrid;
class EventBus;

enum class TSpinType {
    None = 0,
    Mini,
    Standard
};

class ActivePiece {
public:
    ActivePiece() = default;

    void Spawn(TetrominoType type, GridCoord startPos, CellType minoType = CellType::Solid, float elasticity = 1.0f);

    bool TryMove(GridCoord delta, const IGrid& grid, EventBus* eventBus = nullptr);
    bool TryRotate(int direction, const IGrid& grid, EventBus* eventBus = nullptr); // +1 CW, -1 CCW, +2 180°
    int HardDrop(const IGrid& grid, EventBus* eventBus = nullptr);

    void Update(float dt, float fallInterval, const IGrid& grid, EventBus* eventBus = nullptr);

    [[nodiscard]] std::array<GridCoord, 4> GetMinoGridCoords() const noexcept;
    [[nodiscard]] std::array<GridCoord, 4> GetMinoGridCoordsAt(GridCoord pos, int rot) const noexcept;
    [[nodiscard]] GridCoord GetGhostPosition(const IGrid& grid) const noexcept;

    [[nodiscard]] std::vector<Vector2> GetTargetWorldPositions(const IGrid& grid, Vector2 gridOrigin, float cellSize) const;

    void Render(const IGrid& grid, Vector2 gridOrigin, float cellSize, bool showGhost = true, bool showDebug = false) const;

    [[nodiscard]] bool IsLocked() const noexcept { return m_isLocked; }
    [[nodiscard]] bool IsGrounded() const noexcept { return m_isGrounded; }
    [[nodiscard]] TetrominoType GetType() const noexcept { return m_type; }
    [[nodiscard]] CellType GetMinoType() const noexcept { return m_minoType; }
    [[nodiscard]] GridCoord GetPosition() const noexcept { return m_position; }
    [[nodiscard]] int GetRotation() const noexcept { return m_rotation; }
    [[nodiscard]] SoftBodyMesh& GetSoftBody() noexcept { return m_softBody; }

    [[nodiscard]] TSpinType CheckTSpin(const IGrid& grid) const noexcept;
    [[nodiscard]] bool WasLastActionRotate() const noexcept { return m_lastActionWasRotate; }

    void SetGhostPhase(float durationSeconds) noexcept { m_ghostPhaseTimer = durationSeconds; }
    [[nodiscard]] bool IsGhostPhaseActive() const noexcept { return m_ghostPhaseTimer > 0.0f; }

    void ApplyImpulse(Vector2 impulse) { m_softBody.ApplyImpulse(impulse); }

private:
    [[nodiscard]] bool CheckCollision(GridCoord pos, int rot, const IGrid& grid) const noexcept;

    GridCoord m_position = { 3, -1 };
    int m_rotation = 0;
    TetrominoType m_type = TetrominoType::T;
    CellType m_minoType = CellType::Solid;

    SoftBodyMesh m_softBody;

    float m_fallTimer = 0.0f;
    float m_lockTimer = 0.0f;
    int m_lockMoveCount = 0;
    bool m_isGrounded = false;
    bool m_isLocked = false;
    float m_ghostPhaseTimer = 0.0f;
    bool m_lastActionWasRotate = false;
    size_t m_lastKickIndex = 0;
};

} // namespace TetroShift
