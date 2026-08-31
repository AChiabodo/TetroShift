#pragma once
#include <cstdint>
#include <raylib.h>
#include "core/Constants.hpp"

namespace TetroShift {

enum class TetrominoType : uint8_t {
    I = 0,
    J,
    L,
    O,
    S,
    T,
    Z,
    Count,
    None = 255
};

[[nodiscard]] constexpr Color GetTetrominoColor(TetrominoType type) noexcept {
    switch (type) {
        case TetrominoType::I: return Colors::PieceI;
        case TetrominoType::J: return Colors::PieceJ;
        case TetrominoType::L: return Colors::PieceL;
        case TetrominoType::O: return Colors::PieceO;
        case TetrominoType::S: return Colors::PieceS;
        case TetrominoType::T: return Colors::PieceT;
        case TetrominoType::Z: return Colors::PieceZ;
        default:               return WHITE;
    }
}

[[nodiscard]] constexpr const char* GetTetrominoName(TetrominoType type) noexcept {
    switch (type) {
        case TetrominoType::I: return "I-Beam";
        case TetrominoType::J: return "J-Hook";
        case TetrominoType::L: return "L-Bracket";
        case TetrominoType::O: return "O-Cube";
        case TetrominoType::S: return "S-Snake";
        case TetrominoType::T: return "T-Spike";
        case TetrominoType::Z: return "Z-Zigzag";
        default:               return "Unknown";
    }
}

} // namespace TetroShift
