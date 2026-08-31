#pragma once
#include "IGameState.hpp"
#include "roguelike/Card.hpp"
#include <vector>
#include <string>

namespace TetroShift {

struct ShopConsumable {
    std::string id;
    std::string title;
    std::string description;
    int cost;
    Color color;
};

class InRunShopState : public IGameState {
public:
    explicit InRunShopState(int nextFloorNumber);

    void OnEnter(GameApp& app) override;
    void OnExit(GameApp& app) override;
    void Update(GameApp& app, float dt) override;
    void Render(GameApp& app) override;
    void HandleInput(GameApp& app) override;

private:
    void GenerateStock(GameApp& app);
    void BuyCard(GameApp& app, size_t index);
    void PurgeCard(GameApp& app, size_t index);
    void UpgradeCard(GameApp& app, size_t index);
    void BuyConsumable(GameApp& app, size_t index);
    void CloseShop(GameApp& app);

    int m_nextFloorNumber = 2;
    int m_activeSection = 0; // 0: Cards, 1: Purge, 2: Armory, 3: Overcharge
    int m_selectedItemIndex = 0;
    float m_animTimer = 0.0f;

    std::vector<Card> m_marketCards;
    std::vector<ShopConsumable> m_armoryItems;
};

} // namespace TetroShift
