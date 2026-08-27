#pragma once
#include "Card.hpp"
#include "Inventory.hpp"
#include <vector>
#include <memory>
#include <random>

namespace TetroShift {

class CardDatabase {
public:
    CardDatabase();

    [[nodiscard]] const std::vector<Card>& GetAllCards() const noexcept { return m_cards; }
    [[nodiscard]] const Card* FindCardById(const std::string& id) const;

    [[nodiscard]] std::vector<Card> GenerateDraftChoices(size_t count, const Inventory& currentInventory, int floorNumber);
    [[nodiscard]] std::vector<Card> GenerateShopChoices(size_t count, const Inventory& currentInventory, int floorNumber);

private:
    void RegisterAllCards();

    std::vector<Card> m_cards;
    std::mt19937 m_rng;
};

} // namespace TetroShift
