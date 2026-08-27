#pragma once
#include "Inventory.hpp"
#include "Card.hpp"
#include <vector>
#include <string>

namespace TetroShift {

class RunManager {
public:
    RunManager();

    void Reset();

    void AddScore(int basePoints, const std::string& reason = "");
    void RegisterLineClear(int linesCount, bool isTetris, const CardContext& ctx);
    void RegisterPieceSpawn(const CardContext& ctx);
    void RegisterPieceLock(const CardContext& ctx);

    void AdvanceFloor();

    // Getters & Setters
    [[nodiscard]] int GetScore() const noexcept { return m_score; }
    [[nodiscard]] int GetFloor() const noexcept { return m_floor; }
    [[nodiscard]] int GetLinesTotal() const noexcept { return m_linesTotal; }
    [[nodiscard]] int GetLinesThisFloor() const noexcept { return m_linesThisFloor; }
    [[nodiscard]] int GetFloorLineTarget() const noexcept { return m_floorLineTarget; }
    [[nodiscard]] int GetCombo() const noexcept { return m_combo; }
    [[nodiscard]] float GetScoreMultiplier() const noexcept { return m_scoreMultiplier; }
    [[nodiscard]] float GetFallInterval() const noexcept;
    [[nodiscard]] bool IsDraftPending() const noexcept { return m_draftPending; }
    void ConsumeDraftFlag() noexcept { m_draftPending = false; }
    void TriggerDraft() noexcept { m_draftPending = true; }

    [[nodiscard]] bool IsGameOver() const noexcept { return m_isGameOver; }
    void SetGameOver(bool over) noexcept { m_isGameOver = over; }

    [[nodiscard]] Inventory& GetInventory() noexcept { return m_inventory; }
    [[nodiscard]] const Inventory& GetInventory() const noexcept { return m_inventory; }

    void MultiplyScoreModifier(float factor) noexcept { m_scoreMultiplier *= factor; }
    void SetBaseFallSpeedMultiplier(float factor) noexcept { m_speedMultiplier = factor; }

    // Physical elastic global modifier
    void SetGlobalElasticity(float e) noexcept { m_globalElasticity = e; }
    [[nodiscard]] float GetGlobalElasticity() const noexcept { return m_globalElasticity; }

private:
    int m_score = 0;
    int m_floor = 1;
    int m_linesTotal = 0;
    int m_linesThisFloor = 0;
    int m_floorLineTarget = 6;
    int m_combo = -1;
    float m_scoreMultiplier = 1.0f;
    float m_speedMultiplier = 1.0f;
    float m_globalElasticity = 1.0f;
    bool m_draftPending = false;
    bool m_isGameOver = false;

    Inventory m_inventory;
};

} // namespace TetroShift
