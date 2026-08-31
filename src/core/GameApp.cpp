#include "GameApp.hpp"
#include "Constants.hpp"
#include "states/TitleState.hpp"
#include "states/PlayState.hpp"
#include <raylib.h>

namespace TetroShift {

// Static global instance pointer for bridge if needed
static PlayState* g_activePlayState = nullptr;

GameApp::GameApp() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetExitKey(KEY_NULL); // Prevent Raylib from closing window directly on ESC
    SetTargetFPS(60);

    InitAudioDevice();
    m_soundSynth.Initialize();
    m_musicManager.Initialize();
    m_saveManager.Initialize();

    m_stateManager.SetState(*this, std::make_unique<TitleState>());
}

GameApp::~GameApp() {
    m_musicManager.Shutdown();
    m_soundSynth.Shutdown();
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
    if (IsWindowReady()) {
        CloseWindow();
    }
}

void GameApp::Run() {
    m_isRunning = true;
    while (!WindowShouldClose() && m_isRunning) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f; // Clamp frame delta on lag spikes

        m_musicManager.Update(dt);
        m_stateManager.HandleInput(*this);
        if (!m_isRunning) break;

        m_stateManager.Update(*this, dt);
        if (!m_isRunning) break;

        BeginDrawing();
        m_stateManager.Render(*this);
        EndDrawing();
    }
}

#include "states/PlayState.hpp"

PlayState* GameApp::GetActivePlayState() {
    return dynamic_cast<PlayState*>(m_stateManager.GetCurrentState());
}

const Inventory& GameApp::GetPlayStateInventory() const {
    static Inventory fallback;
    auto* current = const_cast<GameStateManager&>(m_stateManager).GetCurrentState();
    auto* play = dynamic_cast<PlayState*>(current);
    if (play) {
        return play->GetRunManager().GetInventory();
    }
    return fallback;
}

Inventory* GameApp::GetPlayStateInventoryMut() {
    auto* play = GetActivePlayState();
    if (play) {
        return &play->GetRunManager().GetInventory();
    }
    return nullptr;
}

void GameApp::ApplyDraftCard(const Card& card) {
    auto* play = GetActivePlayState();
    if (play) {
        CardContext ctx{ &play->GetRunManager(), &play->GetActivePiece(), &play->GetGrid(), &play->GetSpawner(), &m_eventBus };
        play->GetRunManager().GetInventory().AddCard(card, ctx);
    }
    m_eventBus.Publish(EventCardAcquired{ card.id });
}

bool GameApp::UseRerollToken() {
    auto* play = GetActivePlayState();
    if (play) {
        return play->GetRunManager().GetInventory().UseReroll();
    }
    return false;
}

int GameApp::GetRemainingRerolls() const {
    auto* current = const_cast<GameStateManager&>(m_stateManager).GetCurrentState();
    auto* play = dynamic_cast<PlayState*>(current);
    if (play) {
        return play->GetRunManager().GetInventory().GetRerolls();
    }
    return 0;
}

} // namespace TetroShift
