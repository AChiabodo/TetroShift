#pragma once
#include <raylib.h>
#include <cstdint>

namespace TetroShift {

enum class CellType : uint8_t {
    Empty = 0,
    Solid,
    Jelly,
    HeavyIron,
    Bomb,
    Gold,
    QuantumGhost,
    Glitch
};

struct Cell {
    CellType type = CellType::Empty;
    Color color = { 0, 0, 0, 0 };
    float elasticity = 1.0f;
    float squishOffset = 0.0f;
    float flashTimer = 0.0f;
    bool isLocked = false;
    uint8_t sourcePieceId = 0;

    [[nodiscard]] constexpr bool IsEmpty() const noexcept {
        return type == CellType::Empty;
    }

    [[nodiscard]] constexpr bool IsSolid() const noexcept {
        return type != CellType::Empty;
    }

    void Clear() noexcept {
        type = CellType::Empty;
        color = { 0, 0, 0, 0 };
        elasticity = 1.0f;
        squishOffset = 0.0f;
        flashTimer = 0.0f;
        isLocked = false;
        sourcePieceId = 0;
    }
};

} // namespace TetroShift
