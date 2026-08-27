#pragma once
#include "IGameState.hpp"
#include <raylib.h>

namespace TetroShift {

class TitleState : public IGameState {
public:
    TitleState() = default;

    void OnEnter(GameApp& app) override;
    void OnExit(GameApp& app) override;
    void Update(GameApp& app, float dt) override;
    void Render(GameApp& app) override;
    void HandleInput(GameApp& app) override;

private:
    float m_animTimer = 0.0f;
    int m_selectedOption = 0;
};

} // namespace TetroShift
