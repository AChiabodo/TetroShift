#pragma once
#include <string>
#include <vector>
#include <raylib.h>
#include "piece/TetrominoType.hpp"

namespace TetroShift {

// Active sub-view within the Main Menu
enum class MenuView {
    Main,
    PlaySelect,
    ProfileSaves,
    HighScores,
    Shop,
    Customization,
    Settings,
    QuitConfirm
};

// Global configurable settings
struct GameSettings {
    float masterVolume = 0.8f;
    bool isMuted = false;
    int screenShakeLevel = 2;       // 0: Off, 1: Subtle, 2: Normal, 3: Heavy
    bool crtScanlines = true;
    int softBodyWobble = 1;         // 0: Minimal, 1: Standard, 2: Ultra Jelly
    float ghostPieceAlpha = 0.35f;
    bool fastDAS = false;           // Quick Delayed Auto Shift mode
};

// Profile & Career stats
struct PlayerProfileData {
    std::string pilotName = "CYBER_PILOT_01";
    std::string pilotCallsign = "MORPHO-PRIME";
    std::string rankTitle = "MASTER ARCHITECT III";
    int level = 14;
    int currentExp = 3850;
    int maxExp = 5000;
    int totalRuns = 42;
    int totalVictories = 18;
    int totalLinesCleared = 2450;
    int highestScore = 184200;
    int highestFloor = 12;
    int energyCredits = 1450; // Currency for Shop
};

// Save Slot model for future load/save subsystem
enum class SaveSlotState {
    Empty,
    ActiveRun,
    CompletedRun
};

struct SaveSlotData {
    int slotId = 1;
    SaveSlotState state = SaveSlotState::Empty;
    std::string runMode = "ROGUELIKE RUN";
    int currentFloor = 1;
    int currentScore = 0;
    int currentLines = 0;
    int energyCoins = 0;
    std::string timestamp = "--/--/----";
    std::vector<std::string> relicCards;
    Color accentColor = { 0, 240, 255, 255 };
};

// High score record entry
struct HighScoreEntry {
    int rank = 1;
    std::string pilotName = "PILOT";
    int score = 0;
    int floorReached = 1;
    int linesCleared = 0;
    std::string date = "2026-08-30";
    std::string badge = "LEGEND";
    Color badgeColor = { 255, 215, 0, 255 };
};

// Black Market Shop Item
enum class ShopCategory {
    Relic,
    Skin,
    Booster,
    MatrixTheme
};

struct ShopItemData {
    std::string id;
    std::string name;
    ShopCategory category = ShopCategory::Relic;
    std::string categoryName;
    std::string description;
    int cost = 100;
    bool isUnlocked = false;
    bool isEquipped = false;
    Color rarityColor = { 0, 240, 255, 255 };
};

// Customization skins & themes
struct CustomThemeData {
    std::string id;
    std::string name;
    std::string description;
    Color primaryColor;
    Color secondaryColor;
    TetrominoType previewType = TetrominoType::T;
    bool isUnlocked = true;
    bool isEquipped = false;
};

} // namespace TetroShift
