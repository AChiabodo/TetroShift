#pragma once
#include "TetrominoType.hpp"
#include "grid/Cell.hpp"
#include <vector>
#include <deque>
#include <optional>
#include <random>

namespace TetroShift {

class PieceSpawner {
public:
    explicit PieceSpawner(uint32_t seed = 1337);

    void Reset(uint32_t seed = 1337);

    [[nodiscard]] TetrominoType PopNext();
    [[nodiscard]] std::vector<TetrominoType> PeekNextQueue(size_t count = 5) const;

    // Hold piece mechanics
    bool TryHold(TetrominoType current, std::optional<TetrominoType>& outSwappedPiece);
    void UnlockHold() noexcept { m_canHoldThisDrop = true; }
    [[nodiscard]] const std::optional<TetrominoType>& GetHoldPiece() const noexcept { return m_holdPiece; }
    [[nodiscard]] const std::optional<TetrominoType>& GetSecondHoldPiece() const noexcept { return m_secondHoldPiece; }
    [[nodiscard]] bool CanHold() const noexcept { return m_canHoldThisDrop; }

    // Modifiers / Upgrades
    void SetDoubleHold(bool enabled) noexcept { m_doubleHoldUnlocked = enabled; }
    void SetMidasFrequency(int everyNPieces) noexcept { m_midasFrequency = everyNPieces; }
    void SetBombChance(float chance) noexcept { m_bombChance = chance; }
    void SetJellyChance(float chance) noexcept { m_jellyChance = chance; }

    [[nodiscard]] CellType DetermineSpawnCellType();

private:
    void GenerateBag();

    std::mt19937 m_rng;
    std::deque<TetrominoType> m_queue;

    std::optional<TetrominoType> m_holdPiece;
    std::optional<TetrominoType> m_secondHoldPiece;
    bool m_canHoldThisDrop = true;
    bool m_doubleHoldUnlocked = false;

    int m_piecesSpawnedCount = 0;
    int m_midasFrequency = 0; // 0 = disabled, >0 = every N pieces
    float m_bombChance = 0.0f;
    float m_jellyChance = 0.0f;
};

} // namespace TetroShift
