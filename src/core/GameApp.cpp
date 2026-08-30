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

const Inventory& GameApp::GetPlayStateInventory() const {
    static Inventory fallback;
    return fallback;
}

void GameApp::ApplyDraftCard(const Card& card) {
    // In our state hierarchy, PlayState is the base state under the Draft overlay
    // We will publish the event and advance
    m_eventBus.Publish(EventCardAcquired{ card.id });
}

bool GameApp::UseRerollToken() {
    return true;
}

int GameApp::GetRemainingRerolls() const {
    return 1;
}

} // namespace TetroShift
