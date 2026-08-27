#pragma once
#include "TetrominoType.hpp"
#include "grid/GridCoord.hpp"
#include <array>
#include <span>

namespace TetroShift {

struct TetrominoShape {
    // 4 rotations, each has 4 minos
    std::array<std::array<GridCoord, 4>, 4> rotations;
};

class TetrominoDefinition {
public:
    [[nodiscard]] static constexpr std::array<GridCoord, 4> GetMinoCoords(TetrominoType type, int rotation) noexcept {
        const int rot = (rotation % 4 + 4) % 4;
        switch (type) {
            case TetrominoType::I:
                // 4x4 bounding box
                if (rot == 0) return {{ {0, 1}, {1, 1}, {2, 1}, {3, 1} }};
                if (rot == 1) return {{ {2, 0}, {2, 1}, {2, 2}, {2, 3} }};
                if (rot == 2) return {{ {0, 2}, {1, 2}, {2, 2}, {3, 2} }};
                return               {{ {1, 0}, {1, 1}, {1, 2}, {1, 3} }};

            case TetrominoType::J:
                if (rot == 0) return {{ {0, 0}, {0, 1}, {1, 1}, {2, 1} }};
                if (rot == 1) return {{ {1, 0}, {2, 0}, {1, 1}, {1, 2} }};
                if (rot == 2) return {{ {0, 1}, {1, 1}, {2, 1}, {2, 2} }};
                return               {{ {1, 0}, {1, 1}, {0, 2}, {1, 2} }};

            case TetrominoType::L:
                if (rot == 0) return {{ {2, 0}, {0, 1}, {1, 1}, {2, 1} }};
                if (rot == 1) return {{ {1, 0}, {1, 1}, {1, 2}, {2, 2} }};
                if (rot == 2) return {{ {0, 1}, {1, 1}, {2, 1}, {0, 2} }};
                return               {{ {0, 0}, {1, 0}, {1, 1}, {1, 2} }};

            case TetrominoType::O:
                // O piece does not rotate relative to 2x2 box
                return {{ {1, 0}, {2, 0}, {1, 1}, {2, 1} }};

            case TetrominoType::S:
                if (rot == 0) return {{ {1, 0}, {2, 0}, {0, 1}, {1, 1} }};
                if (rot == 1) return {{ {1, 0}, {1, 1}, {2, 1}, {2, 2} }};
                if (rot == 2) return {{ {1, 1}, {2, 1}, {0, 2}, {1, 2} }};
                return               {{ {0, 0}, {0, 1}, {1, 1}, {1, 2} }};

            case TetrominoType::T:
                if (rot == 0) return {{ {1, 0}, {0, 1}, {1, 1}, {2, 1} }};
                if (rot == 1) return {{ {1, 0}, {1, 1}, {2, 1}, {1, 2} }};
                if (rot == 2) return {{ {0, 1}, {1, 1}, {2, 1}, {1, 2} }};
                return               {{ {1, 0}, {0, 1}, {1, 1}, {1, 2} }};

            case TetrominoType::Z:
                if (rot == 0) return {{ {0, 0}, {1, 0}, {1, 1}, {2, 1} }};
                if (rot == 1) return {{ {2, 0}, {1, 1}, {2, 1}, {1, 2} }};
                if (rot == 2) return {{ {0, 1}, {1, 1}, {1, 2}, {2, 2} }};
                return               {{ {1, 0}, {0, 1}, {1, 1}, {0, 2} }};

            default:
                return {{ {0, 0}, {0, 0}, {0, 0}, {0, 0} }};
        }
    }

    [[nodiscard]] static constexpr std::array<GridCoord, 5> GetWallKicks(TetrominoType type, int fromRot, int toRot) noexcept {
        const int from = (fromRot % 4 + 4) % 4;
        const int to = (toRot % 4 + 4) % 4;

        if (type == TetrominoType::I) {
            if (from == 0 && to == 1) return {{ GridCoord{0,0}, GridCoord{-2,0}, GridCoord{+1,0}, GridCoord{-2,+1}, GridCoord{+1,-2} }};
            if (from == 1 && to == 0) return {{ GridCoord{0,0}, GridCoord{+2,0}, GridCoord{-1,0}, GridCoord{+2,-1}, GridCoord{-1,+2} }};
            if (from == 1 && to == 2) return {{ GridCoord{0,0}, GridCoord{-1,0}, GridCoord{+2,0}, GridCoord{-1,-2}, GridCoord{+2,+1} }};
            if (from == 2 && to == 1) return {{ GridCoord{0,0}, GridCoord{+1,0}, GridCoord{-2,0}, GridCoord{+1,+2}, GridCoord{-2,-1} }};
            if (from == 2 && to == 3) return {{ GridCoord{0,0}, GridCoord{+2,0}, GridCoord{-1,0}, GridCoord{+2,-1}, GridCoord{-1,+2} }};
            if (from == 3 && to == 2) return {{ GridCoord{0,0}, GridCoord{-2,0}, GridCoord{+1,0}, GridCoord{-2,+1}, GridCoord{+1,-2} }};
            if (from == 3 && to == 0) return {{ GridCoord{0,0}, GridCoord{+1,0}, GridCoord{-2,0}, GridCoord{+1,+2}, GridCoord{-2,-1} }};
            if (from == 0 && to == 3) return {{ GridCoord{0,0}, GridCoord{-1,0}, GridCoord{+2,0}, GridCoord{-1,-2}, GridCoord{+2,+1} }};
            return {{ GridCoord{0,0}, GridCoord{0,0}, GridCoord{0,0}, GridCoord{0,0}, GridCoord{0,0} }};
        }

        // Standard Pieces (J, L, S, T, Z)
        if (from == 0 && to == 1) return {{ GridCoord{0,0}, GridCoord{-1,0}, GridCoord{-1,-1}, GridCoord{0,+2}, GridCoord{-1,+2} }};
        if (from == 1 && to == 0) return {{ GridCoord{0,0}, GridCoord{+1,0}, GridCoord{+1,+1}, GridCoord{0,-2}, GridCoord{+1,-2} }};
        if (from == 1 && to == 2) return {{ GridCoord{0,0}, GridCoord{+1,0}, GridCoord{+1,+1}, GridCoord{0,-2}, GridCoord{+1,-2} }};
        if (from == 2 && to == 1) return {{ GridCoord{0,0}, GridCoord{-1,0}, GridCoord{-1,-1}, GridCoord{0,+2}, GridCoord{-1,+2} }};
        if (from == 2 && to == 3) return {{ GridCoord{0,0}, GridCoord{+1,0}, GridCoord{+1,-1}, GridCoord{0,+2}, GridCoord{+1,+2} }};
        if (from == 3 && to == 2) return {{ GridCoord{0,0}, GridCoord{-1,0}, GridCoord{-1,+1}, GridCoord{0,-2}, GridCoord{-1,-2} }};
        if (from == 3 && to == 0) return {{ GridCoord{0,0}, GridCoord{-1,0}, GridCoord{-1,+1}, GridCoord{0,-2}, GridCoord{-1,-2} }};
        if (from == 0 && to == 3) return {{ GridCoord{0,0}, GridCoord{+1,0}, GridCoord{+1,-1}, GridCoord{0,+2}, GridCoord{+1,+2} }};

        return {{ GridCoord{0,0}, GridCoord{0,0}, GridCoord{0,0}, GridCoord{0,0}, GridCoord{0,0} }};
    }
};

} // namespace TetroShift
