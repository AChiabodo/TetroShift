#pragma once
#include "Card.hpp"
#include <vector>
#include <string>
#include <algorithm>

namespace TetroShift {

class Inventory {
public:
    Inventory() = default;

    void Reset() {
        m_passiveCards.clear();
        m_activeCards.clear();
        m_coins = 20; // Starting pocket cash
        m_rerollTokens = 1;
    }

    void AddCard(const Card& card, const CardContext& ctx) {
        if (card.category == CardCategory::ActiveAbility) {
            m_activeCards.push_back(card);
        } else {
            m_passiveCards.push_back(card);
        }
        if (card.onAcquire) {
            card.onAcquire(ctx);
        }
    }

    [[nodiscard]] bool HasCard(const std::string& cardId) const noexcept {
        for (const auto& c : m_passiveCards) {
            if (c.id == cardId) return true;
        }
        for (const auto& c : m_activeCards) {
            if (c.id == cardId) return true;
        }
        return false;
    }

    void TickCooldowns(int linesCleared) {
        for (auto& active : m_activeCards) {
            if (active.currentCooldown > 0) {
                active.currentCooldown = std::max(0, active.currentCooldown - linesCleared);
            }
        }
    }

    bool TryUseActiveAbility(size_t index, const CardContext& ctx) {
        if (index >= m_activeCards.size()) return false;
        auto& active = m_activeCards[index];
        if (active.currentCooldown == 0 && active.onActiveUse) {
            if (active.onActiveUse(ctx)) {
                active.currentCooldown = active.maxCooldown;
                return true;
            }
        }
        return false;
    }

    void AddCoins(int amount) noexcept { m_coins += amount; }
    bool SpendCoins(int amount) noexcept {
        if (m_coins >= amount) {
            m_coins -= amount;
            return true;
        }
        return false;
    }

    [[nodiscard]] int GetCoins() const noexcept { return m_coins; }
    void SetCoins(int coins) noexcept { m_coins = coins; }
    [[nodiscard]] int GetRerolls() const noexcept { return m_rerollTokens; }
    void SetRerolls(int r) noexcept { m_rerollTokens = r; }
    void AddRerolls(int count) noexcept { m_rerollTokens += count; }
    bool UseReroll() noexcept {
        if (m_rerollTokens > 0) {
            m_rerollTokens--;
            return true;
        }
        return false;
    }

    void ClearCards() noexcept {
        m_passiveCards.clear();
        m_activeCards.clear();
    }

    [[nodiscard]] const std::vector<Card>& GetPassives() const noexcept { return m_passiveCards; }
    [[nodiscard]] const std::vector<Card>& GetActives() const noexcept { return m_activeCards; }

private:
    std::vector<Card> m_passiveCards;
    std::vector<Card> m_activeCards;
    int m_coins = 20;
    int m_rerollTokens = 1;
};

} // namespace TetroShift
