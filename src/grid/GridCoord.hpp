#pragma once
#include <functional>
#include <compare>

namespace TetroShift {

struct GridCoord {
    int x = 0;
    int y = 0;

    constexpr GridCoord() noexcept = default;
    constexpr GridCoord(int inX, int inY) noexcept : x(inX), y(inY) {}

    constexpr auto operator<=>(const GridCoord&) const noexcept = default;
    constexpr bool operator==(const GridCoord&) const noexcept = default;

    constexpr GridCoord operator+(const GridCoord& other) const noexcept {
        return { x + other.x, y + other.y };
    }

    constexpr GridCoord operator-(const GridCoord& other) const noexcept {
        return { x - other.x, y - other.y };
    }

    constexpr GridCoord& operator+=(const GridCoord& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr GridCoord& operator-=(const GridCoord& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }
};

} // namespace TetroShift

// Standard hash specialization for GridCoord
template <>
struct std::hash<TetroShift::GridCoord> {
    std::size_t operator()(const TetroShift::GridCoord& c) const noexcept {
        return (static_cast<std::size_t>(c.x) << 16) ^ (static_cast<std::size_t>(c.y) & 0xFFFF);
    }
};
