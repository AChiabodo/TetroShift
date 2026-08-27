#pragma once
#include "IGameState.hpp"
#include "roguelike/Card.hpp"
#include <vector>

namespace TetroShift {

class CardDraftState : public IGameState {
public:
    explicit CardDraftState(int floorNumber);

    void OnEnter(GameApp& app) override;
    void OnExit(GameApp& app) override;
    void Update(GameApp& app, float dt) override;
    void Render(GameApp& app) override;
    void HandleInput(GameApp& app) override;

private:
    void SelectCard(GameApp& app, size_t index);
    void Reroll(GameApp& app);

    int m_floorNumber = 1;
    std::vector<Card> m_choices;
    int m_hoveredIndex = -1;
    int m_selectedIndex = 0;
    float m_animTimer = 0.0f;
};

} // namespace TetroShift
