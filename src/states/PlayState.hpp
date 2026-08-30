#pragma once
#include "IGameState.hpp"
#include "grid/OrthogonalGrid.hpp"
#include "piece/ActivePiece.hpp"
#include "piece/PieceSpawner.hpp"
#include "roguelike/RunManager.hpp"
#include "render/ParticleSystem.hpp"
#include "render/ScreenEffects.hpp"
#include "render/Renderer.hpp"
#include "render/MenuRenderer.hpp"
#include <memory>

namespace TetroShift {

class PlayState : public IGameState {
public:
    PlayState();
    ~PlayState() override = default;

    void OnEnter(GameApp& app) override;
    void OnExit(GameApp& app) override;
    void Update(GameApp& app, float dt) override;
    void Render(GameApp& app) override;
    void HandleInput(GameApp& app) override;

    void SpawnNextPiece(GameApp& app);
    void HandlePieceLock(GameApp& app);

    [[nodiscard]] IGrid& GetGrid() noexcept { return *m_grid; }
    [[nodiscard]] ActivePiece& GetActivePiece() noexcept { return m_activePiece; }
    [[nodiscard]] PieceSpawner& GetSpawner() noexcept { return m_spawner; }
    [[nodiscard]] RunManager& GetRunManager() noexcept { return m_runManager; }
    [[nodiscard]] ParticleSystem& GetParticles() noexcept { return m_particles; }
    [[nodiscard]] ScreenEffects& GetScreenEffects() noexcept { return m_screenEffects; }

private:
    void HandleMovementInput(GameApp& app, float dt);
    void TriggerInstantLineClear(GameApp& app);
    void RenderPauseMenu(GameApp& app);

    std::unique_ptr<IGrid> m_grid;
    ActivePiece m_activePiece;
    PieceSpawner m_spawner;
    RunManager m_runManager;
    ParticleSystem m_particles;
    ScreenEffects m_screenEffects;
    Renderer m_renderer;
    MenuRenderer m_menuRenderer;

    // DAS / ARR state
    float m_dasTimer = 0.0f;
    float m_arrTimer = 0.0f;
    int m_heldDirection = 0; // -1: Left, +1: Right

    bool m_showDebugPhysics = false;
    bool m_isPaused = false;
    int m_pauseSelectedOption = 0;
};

} // namespace TetroShift
