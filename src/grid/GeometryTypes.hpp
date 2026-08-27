#pragma once
#include <cstdint>

namespace TetroShift {

enum class GridGeometry : uint8_t {
    Orthogonal, // Classic 10x20 Cartesian
    Radial,     // Cylindrical / Concentric Circular (Phase 5 extensibility)
    Hexagonal   // Hexagonal honeycomb (Future expansion)
};

} // namespace TetroShift
