#pragma once
#include <raylib.h>
#include <cstdint>

namespace TetroShift {

// Window configuration
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 768;
constexpr const char* WINDOW_TITLE = "TetroShift // MorphoTetris - Roguelike Physics C++20";

// Grid configuration
constexpr int GRID_WIDTH = 10;
constexpr int GRID_HEIGHT = 20;
constexpr int GRID_BUFFER_HEIGHT = 4; // Hidden spawn rows above grid
constexpr int GRID_TOTAL_HEIGHT = GRID_HEIGHT + GRID_BUFFER_HEIGHT;
constexpr float CELL_SIZE = 32.0f;

// Playfield offset on screen
constexpr float PLAYFIELD_X = (WINDOW_WIDTH - (GRID_WIDTH * CELL_SIZE)) * 0.5f; // Centered
constexpr float PLAYFIELD_Y = (WINDOW_HEIGHT - (GRID_HEIGHT * CELL_SIZE)) * 0.5f + 10.0f;

// Timing & Physics constants
constexpr float BASE_LOCK_DELAY = 0.5f;        // Seconds before active piece locks on floor
constexpr int MAX_LOCK_MOVES = 15;             // Max resets to lock delay before force lock
constexpr float DAS_DELAY = 0.15f;             // Delayed Auto Shift initial delay (seconds)
constexpr float ARR_INTERVAL = 0.033f;         // Auto Repeat Rate repeat interval (seconds)
constexpr float SOFT_DROP_FACTOR = 20.0f;      // Speedup multiplier for soft drop

// Soft-Body Spring Physics constants
constexpr float SPRING_STIFFNESS_DEFAULT = 140.0f;
constexpr float SPRING_DAMPING_DEFAULT = 7.5f;
constexpr float WOBBLE_IMPULSE_MOVE = 8.0f;
constexpr float WOBBLE_IMPULSE_ROTATE = 16.0f;
constexpr float WOBBLE_IMPULSE_DROP = 24.0f;

// Color Palette (Cyberpunk / Neon Synth aesthetic)
namespace Colors {
    // UI & Backgrounds
    constexpr Color BgDark         = { 12, 14, 22, 255 };
    constexpr Color BgPanel        = { 20, 24, 38, 240 };
    constexpr Color BgPanelBorder  = { 45, 55, 85, 255 };
    constexpr Color GridBg         = { 16, 18, 28, 255 };
    constexpr Color GridLine       = { 30, 36, 56, 180 };
    constexpr Color GhostPiece     = { 255, 255, 255, 50 };

    // Standard Tetromino Colors (Bright Neon)
    constexpr Color PieceI         = { 0, 240, 255, 255 };    // Cyan
    constexpr Color PieceJ         = { 40, 100, 255, 255 };   // Deep Blue
    constexpr Color PieceL         = { 255, 140, 0, 255 };    // Orange
    constexpr Color PieceO         = { 255, 220, 0, 255 };    // Yellow
    constexpr Color PieceS         = { 0, 255, 120, 255 };    // Lime Green
    constexpr Color PieceT         = { 180, 40, 255, 255 };   // Purple
    constexpr Color PieceZ         = { 255, 40, 80, 255 };    // Neon Red

    // Special Cell Modifiers Colors
    constexpr Color PieceBomb      = { 255, 70, 0, 255 };     // Fiery Orange-Red
    constexpr Color PieceGold      = { 255, 215, 0, 255 };    // Shiny Gold
    constexpr Color PieceJelly     = { 0, 255, 200, 255 };    // Aquamarine Jelly
    constexpr Color PieceIron      = { 160, 170, 190, 255 };  // Metallic Silver

    // Card Rarity Colors
    constexpr Color RarityCommon   = { 150, 170, 190, 255 };  // Grayish Cyan
    constexpr Color RarityRare     = { 60, 160, 255, 255 };   // Royal Blue
    constexpr Color RarityEpic     = { 190, 60, 255, 255 };   // Violet
    constexpr Color RarityLegendary= { 255, 195, 20, 255 };   // Golden Sun
    constexpr Color RarityCursed   = { 255, 40, 60, 255 };    // Blood Red

    // Text & Accents
    constexpr Color TextWhite      = { 240, 244, 255, 255 };
    constexpr Color TextDim        = { 130, 145, 175, 255 };
    constexpr Color TextAccent     = { 0, 240, 255, 255 };
    constexpr Color TextGold       = { 255, 215, 0, 255 };
    constexpr Color TextGreen      = { 80, 255, 140, 255 };
    constexpr Color TextDanger     = { 255, 70, 90, 255 };
}

} // namespace TetroShift
