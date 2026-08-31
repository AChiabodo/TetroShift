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
        m_deflectorShields = 0;
        m_bombCharges = 0;
        m_cryoSpeedBuff = false;
    }

    void AddCard(const Card& card, const CardContext& ctx) {
        if (card.category == CardCategory::ActiveAbility) {
            // Keep at most 2 active abilities (replace oldest if full)
            if (m_activeCards.size() >= 2) {
                m_activeCards[1] = card;
            } else {
                m_activeCards.push_back(card);
            }
        } else {
            m_passiveCards.push_back(card);
        }
        if (card.onAcquire) {
            card.onAcquire(ctx);
        }
    }

    bool RemoveCard(const std::string& cardId) {
        auto itP = std::remove_if(m_passiveCards.begin(), m_passiveCards.end(), [&](const Card& c) {
            return c.id == cardId;
        });
        if (itP != m_passiveCards.end()) {
            m_passiveCards.erase(itP, m_passiveCards.end());
            return true;
        }

        auto itA = std::remove_if(m_activeCards.begin(), m_activeCards.end(), [&](const Card& c) {
            return c.id == cardId;
        });
        if (itA != m_activeCards.end()) {
            m_activeCards.erase(itA, m_activeCards.end());
            return true;
        }
        return false;
    }

    bool UpgradeCard(const std::string& cardId) {
        for (auto& c : m_passiveCards) {
            if (c.id == cardId) {
                c.title += " +";
                c.cost = static_cast<int>(c.cost * 1.5f);
                return true;
            }
        }
        for (auto& c : m_activeCards) {
            if (c.id == cardId) {
                c.title += " +";
                if (c.maxCooldown > 2) c.maxCooldown -= 1;
                return true;
            }
        }
        return false;
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

    // Deflector Shield (Anti-Game Over)
    void AddShield(int count = 1) noexcept { m_deflectorShields += count; }
    [[nodiscard]] int GetShieldCount() const noexcept { return m_deflectorShields; }
    bool ConsumeShield() noexcept {
        if (m_deflectorShields > 0) {
            m_deflectorShields--;
            return true;
        }
        return false;
    }

    // Consumable Buffs
    void AddBombCharges(int count) noexcept { m_bombCharges += count; }
    [[nodiscard]] int GetBombCharges() const noexcept { return m_bombCharges; }
    bool ConsumeBombCharge() noexcept {
        if (m_bombCharges > 0) {
            m_bombCharges--;
            return true;
        }
        return false;
    }

    void SetCryoBuff(bool active) noexcept { m_cryoSpeedBuff = active; }
    [[nodiscard]] bool HasCryoBuff() const noexcept { return m_cryoSpeedBuff; }

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
    [[nodiscard]] std::vector<Card>& GetPassives() noexcept { return m_passiveCards; }
    [[nodiscard]] std::vector<Card>& GetActives() noexcept { return m_activeCards; }

private:
    std::vector<Card> m_passiveCards;
    std::vector<Card> m_activeCards;
    int m_coins = 20;
    int m_rerollTokens = 1;
    int m_deflectorShields = 0;
    int m_bombCharges = 0;
    bool m_cryoSpeedBuff = false;
};

} // namespace TetroShift
