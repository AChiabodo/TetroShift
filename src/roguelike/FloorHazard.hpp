#pragma once
#include <string>
#include <cstdint>
#include <raylib.h>
#include "core/Constants.hpp"

namespace TetroShift {

enum class HazardType : uint8_t {
    None = 0,
    SolarFlare,       // Floor 2: High drop speed + bonus score multiplier
    GravityFlux,      // Floor 5 (Boss 1): Cyclical gravity pulses (2.5x speed spikes & lateral drift)
    CryoChamber,      // Floor 7: Slippery soft-body wobble & slower lock delay
    SingularityCore,  // Floor 10 (Boss 2): Periodic garbage row injection with titanium/obsidian blocks
    MagneticStorm,    // Floor 12: Unstable lateral forces on falling pieces
    GlitchMatrix      // Floor 15 (Boss 3 / Void): Screen glitch pulses & temporary command inversion
};

struct FloorHazardConfig {
    HazardType type = HazardType::None;
    std::string title = "NORMAL SECTOR";
    std::string warningDesc = "Standard gravitational equilibrium";
    Color themeColor = Colors::TextWhite;
    float pulseInterval = 6.0f; // Seconds between hazard pulses
    bool isBossSector = false;
};

} // namespace TetroShift
