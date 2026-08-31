#pragma once
#include "IGameState.hpp"
#include <memory>
#include <vector>

namespace TetroShift {

class GameStateManager {
public:
    GameStateManager() = default;
    ~GameStateManager() = default;

    void SetState(GameApp& app, std::unique_ptr<IGameState> newState);
    void PushOverlay(GameApp& app, std::unique_ptr<IGameState> overlayState);
    void PopOverlay(GameApp& app);

    void Update(GameApp& app, float dt);
    void Render(GameApp& app);
    void HandleInput(GameApp& app);

    [[nodiscard]] bool HasOverlay() const noexcept { return !m_overlayStack.empty(); }
    [[nodiscard]] IGameState* GetCurrentState() noexcept { return m_currentState.get(); }

private:
    std::unique_ptr<IGameState> m_currentState;
    std::vector<std::unique_ptr<IGameState>> m_overlayStack;
};

} // namespace TetroShift
