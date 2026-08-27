#pragma once
#include "IGameState.hpp"
#include <raylib.h>

namespace TetroShift {

class GameOverState : public IGameState {
public:
    GameOverState(int score, int floor, int lines);

    void OnEnter(GameApp& app) override;
    void OnExit(GameApp& app) override;
    void Update(GameApp& app, float dt) override;
    void Render(GameApp& app) override;
    void HandleInput(GameApp& app) override;

private:
    int m_finalScore = 0;
    int m_finalFloor = 1;
    int m_finalLines = 0;
    float m_animTimer = 0.0f;
    int m_selectedOption = 0;
};

} // namespace TetroShift
