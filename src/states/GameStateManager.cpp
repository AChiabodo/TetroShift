#include "GameStateManager.hpp"
#include "core/GameApp.hpp"

namespace TetroShift {

void GameStateManager::SetState(GameApp& app, std::unique_ptr<IGameState> newState) {
    while (!m_overlayStack.empty()) {
        m_overlayStack.back()->OnExit(app);
        m_overlayStack.pop_back();
    }

    if (m_currentState) {
        m_currentState->OnExit(app);
    }

    m_currentState = std::move(newState);

    if (m_currentState) {
        m_currentState->OnEnter(app);
    }
}

void GameStateManager::PushOverlay(GameApp& app, std::unique_ptr<IGameState> overlayState) {
    if (overlayState) {
        overlayState->OnEnter(app);
        m_overlayStack.push_back(std::move(overlayState));
    }
}

void GameStateManager::PopOverlay(GameApp& app) {
    if (!m_overlayStack.empty()) {
        m_overlayStack.back()->OnExit(app);
        m_overlayStack.pop_back();
    }
}

void GameStateManager::Update(GameApp& app, float dt) {
    if (!m_overlayStack.empty()) {
        m_overlayStack.back()->Update(app, dt);
    } else if (m_currentState) {
        m_currentState->Update(app, dt);
    }
}

void GameStateManager::Render(GameApp& app) {
    // Render base state first
    if (m_currentState) {
        m_currentState->Render(app);
    }

    // Render overlays on top
    for (auto& overlay : m_overlayStack) {
        overlay->Render(app);
    }
}

void GameStateManager::HandleInput(GameApp& app) {
    if (!m_overlayStack.empty()) {
        m_overlayStack.back()->HandleInput(app);
    } else if (m_currentState) {
        m_currentState->HandleInput(app);
    }
}

} // namespace TetroShift
