#include "RunManager.hpp"
#include "core/EventBus.hpp"
#include <cmath>
#include <algorithm>

namespace TetroShift {

RunManager::RunManager() {
    Reset();
}

void RunManager::Reset() {
    m_score = 0;
    m_floor = 1;
    m_linesTotal = 0;
    m_linesThisFloor = 0;
    m_floorLineTarget = 6;
    m_combo = -1;
    m_scoreMultiplier = 1.0f;
    m_speedMultiplier = 1.0f;
    m_globalElasticity = 1.0f;
    m_draftPending = false;
    m_isGameOver = false;
    m_inventory.Reset();
}

void RunManager::AddScore(int basePoints, const std::string& /*reason*/) {
    const int finalPoints = static_cast<int>(std::round(static_cast<float>(basePoints) * m_scoreMultiplier));
    m_score += finalPoints;
}

void RunManager::RegisterLineClear(int linesCount, bool isTetris, const CardContext& ctx) {
    if (linesCount <= 0) {
        m_combo = -1;
        return;
    }

    m_combo++;
    m_linesTotal += linesCount;
    m_linesThisFloor += linesCount;

    // Base scoring formula
    int basePoints = 0;
    switch (linesCount) {
        case 1: basePoints = 100 * m_floor; break;
        case 2: basePoints = 300 * m_floor; break;
        case 3: basePoints = 500 * m_floor; break;
        case 4: basePoints = 800 * m_floor; break;
        default: basePoints = 1000 * m_floor; break;
    }

    if (m_combo > 0) {
        basePoints += 50 * m_combo * m_floor;
    }

    AddScore(basePoints, isTetris ? "TETRIS!" : "LINE CLEAR");

    // Coins gained (1 coin per line, 5 bonus for Tetris)
    int coinsEarned = linesCount + (isTetris ? 5 : 0);
    m_inventory.AddCoins(coinsEarned);

    // Trigger card hooks for passive relics
    for (const auto& card : m_inventory.GetPassives()) {
        if (card.onLineClear) {
            card.onLineClear(ctx, linesCount, isTetris);
        }
    }

    // Tick cooldowns for active cards
    m_inventory.TickCooldowns(linesCount);

    // Floor objective check
    if (m_linesThisFloor >= m_floorLineTarget) {
        m_draftPending = true;
    }
}

void RunManager::RegisterPieceSpawn(const CardContext& ctx) {
    for (const auto& card : m_inventory.GetPassives()) {
        if (card.onPieceSpawn) {
            card.onPieceSpawn(ctx);
        }
    }
}

void RunManager::RegisterPieceLock(const CardContext& ctx) {
    for (const auto& card : m_inventory.GetPassives()) {
        if (card.onPieceLock) {
            card.onPieceLock(ctx);
        }
    }
}

void RunManager::AdvanceFloor() {
    m_floor++;
    m_linesThisFloor = 0;
    m_floorLineTarget = 6 + (m_floor - 1) * 3; // Scaling lines per floor
    m_draftPending = false;
    // Reward bonus coins for clearing a floor
    m_inventory.AddCoins(10 + m_floor * 2);
}

float RunManager::GetFallInterval() const noexcept {
    // Speed curve (smoothly faster with floors)
    const float baseInterval = 0.8f * std::pow(0.88f, static_cast<float>(m_floor - 1));
    const float clampedInterval = std::max(0.08f, baseInterval / std::max(0.1f, m_speedMultiplier));
    return clampedInterval;
}

} // namespace TetroShift
