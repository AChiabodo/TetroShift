#pragma once
#include <string>
#include <functional>
#include <raylib.h>
#include "core/Constants.hpp"

namespace TetroShift {

class RunManager;
class ActivePiece;
class IGrid;
class PieceSpawner;
class EventBus;

enum class CardRarity : uint8_t {
    Common = 0,
    Rare,
    Epic,
    Legendary,
    Cursed
};

enum class CardCategory : uint8_t {
    PassiveRelic,
    ActiveAbility,
    Curse
};

struct CardContext {
    RunManager* runManager = nullptr;
    ActivePiece* activePiece = nullptr;
    IGrid* grid = nullptr;
    PieceSpawner* spawner = nullptr;
    EventBus* eventBus = nullptr;
};

struct Card {
    std::string id;
    std::string title;
    std::string description;
    CardRarity rarity = CardRarity::Common;
    CardCategory category = CardCategory::PassiveRelic;
    int cost = 0; // In coins (for shop)
    int maxCooldown = 0; // For active abilities (in lines cleared)
    int currentCooldown = 0;

    // Callbacks
    std::function<void(const CardContext&)> onAcquire = nullptr;
    std::function<void(const CardContext&)> onPieceSpawn = nullptr;
    std::function<void(const CardContext&)> onPieceLock = nullptr;
    std::function<void(const CardContext&, int linesCount, bool isTetris)> onLineClear = nullptr;
    std::function<bool(const CardContext&)> onActiveUse = nullptr; // Returns true if successfully triggered

    [[nodiscard]] Color GetRarityColor() const noexcept {
        switch (rarity) {
            case CardRarity::Common:    return Colors::RarityCommon;
            case CardRarity::Rare:      return Colors::RarityRare;
            case CardRarity::Epic:      return Colors::RarityEpic;
            case CardRarity::Legendary: return Colors::RarityLegendary;
            case CardRarity::Cursed:    return Colors::RarityCursed;
            default:                    return WHITE;
        }
    }

    [[nodiscard]] const char* GetRarityName() const noexcept {
        switch (rarity) {
            case CardRarity::Common:    return "COMMON";
            case CardRarity::Rare:      return "RARE";
            case CardRarity::Epic:      return "EPIC";
            case CardRarity::Legendary: return "LEGENDARY";
            case CardRarity::Cursed:    return "CURSED";
            default:                    return "UNKNOWN";
        }
    }
};

} // namespace TetroShift
