#include "PieceSpawner.hpp"
#include <algorithm>
#include <array>

namespace TetroShift {

PieceSpawner::PieceSpawner(uint32_t seed) : m_rng(seed) {
    Reset(seed);
}

void PieceSpawner::Reset(uint32_t seed) {
    m_rng.seed(seed);
    m_queue.clear();
    m_holdPiece = std::nullopt;
    m_secondHoldPiece = std::nullopt;
    m_canHoldThisDrop = true;
    m_doubleHoldUnlocked = false;
    m_piecesSpawnedCount = 0;
    m_midasFrequency = 0;
    m_bombChance = 0.0f;
    m_jellyChance = 0.0f;

    // Generate initial 2 bags (14 pieces)
    GenerateBag();
    GenerateBag();
}

void PieceSpawner::GenerateBag() {
    std::array<TetrominoType, 7> bag = {
        TetrominoType::I,
        TetrominoType::J,
        TetrominoType::L,
        TetrominoType::O,
        TetrominoType::S,
        TetrominoType::T,
        TetrominoType::Z
    };

    std::shuffle(bag.begin(), bag.end(), m_rng);
    for (auto piece : bag) {
        m_queue.push_back(piece);
    }
}

TetrominoType PieceSpawner::PopNext() {
    if (m_queue.size() < 7) {
        GenerateBag();
    }

    TetrominoType next = m_queue.front();
    m_queue.pop_front();
    m_piecesSpawnedCount++;
    return next;
}

std::vector<TetrominoType> PieceSpawner::PeekNextQueue(size_t count) const {
    std::vector<TetrominoType> peek;
    peek.reserve(count);
    for (size_t i = 0; i < count && i < m_queue.size(); ++i) {
        peek.push_back(m_queue[i]);
    }
    return peek;
}

bool PieceSpawner::TryHold(TetrominoType current, std::optional<TetrominoType>& outSwappedPiece) {
    if (!m_canHoldThisDrop) {
        return false;
    }

    if (!m_holdPiece.has_value()) {
        m_holdPiece = current;
        outSwappedPiece = std::nullopt; // Caller should pop next
    } else {
        outSwappedPiece = m_holdPiece;
        m_holdPiece = current;
    }

    m_canHoldThisDrop = false;
    return true;
}

CellType PieceSpawner::DetermineSpawnCellType() {
    if (m_midasFrequency > 0 && (m_piecesSpawnedCount % m_midasFrequency == 0)) {
        return CellType::Gold;
    }

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(m_rng);

    if (roll < m_bombChance) {
        return CellType::Bomb;
    }
    if (roll < (m_bombChance + m_jellyChance)) {
        return CellType::Jelly;
    }
    if (roll < (m_bombChance + m_jellyChance + m_sandChance)) {
        return CellType::Sand;
    }

    return CellType::Solid;
}

} // namespace TetroShift
