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
#include "roguelike/HazardManager.hpp"
#include "states/MenuTypes.hpp"
#include <memory>
#include <optional>

namespace TetroShift {

class PlayState : public IGameState {
public:
    explicit PlayState(
        GameMode mode = GameMode::Roguelike,
        int activeSlot = 1,
        std::optional<SavedRunState> restoredRun = std::nullopt,
        uint32_t customSeed = 0
    );
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
    [[nodiscard]] HazardManager& GetHazardManager() noexcept { return m_hazardManager; }
    [[nodiscard]] ParticleSystem& GetParticles() noexcept { return m_particles; }
    [[nodiscard]] ScreenEffects& GetScreenEffects() noexcept { return m_screenEffects; }
    [[nodiscard]] int GetActiveSlot() const noexcept { return m_activeSlot; }
    [[nodiscard]] GameMode GetGameMode() const noexcept { return m_gameMode; }
    [[nodiscard]] int GetMarathonLevel() const noexcept { return m_marathonLevel; }

    [[nodiscard]] SavedRunState ExportCurrentRunState() const;

private:
    void HandleMovementInput(GameApp& app, float dt);
    void HandleSandboxInput(GameApp& app);
    void TriggerInstantLineClear(GameApp& app);
    void RenderPauseMenu(GameApp& app);
    void UpdateMarathonSpeed();

    GameMode m_gameMode = GameMode::Roguelike;
    int m_activeSlot = 1;
    std::optional<SavedRunState> m_restoredRun;
    uint32_t m_customSeed = 0;

    // Marathon state
    int m_marathonLevel = 1;
    float m_marathonFallInterval = 1.0f;

    // Sandbox state
    bool m_sandboxZeroGravity = false;
    int m_sandboxSelectedPiece = 0; // 0..6: I,J,L,O,S,T,Z
    int m_sandboxSelectedMinoType = 0; // 0: Normal, 1: Sand, 2: Bomb, 3: Gold, 4: Jelly
    float m_sandboxElasticity = 1.0f;

    std::unique_ptr<IGrid> m_grid;
    ActivePiece m_activePiece;
    PieceSpawner m_spawner;
    RunManager m_runManager;
    HazardManager m_hazardManager;
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
