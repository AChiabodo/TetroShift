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
    m_fontManager.Initialize();
    m_screenEffects.Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Apply persisted settings
    const auto& settings = m_saveManager.GetSettings();
    m_soundSynth.SetMasterVolume(settings.sfxVolume * settings.masterVolume);
    m_musicManager.SetVolume(settings.musicVolume * settings.masterVolume);
    m_screenEffects.SetScanlinesEnabled(settings.crtScanlines);

    // Initialize full-screen offscreen render target for GPU post-processing
    m_screenTexture = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
    SetTextureFilter(m_screenTexture.texture, TEXTURE_FILTER_BILINEAR);

    m_stateManager.SetState(*this, std::make_unique<TitleState>());
}

GameApp::~GameApp() {
    if (m_screenTexture.id > 0) {
        UnloadRenderTexture(m_screenTexture);
        m_screenTexture = {};
    }
    m_screenEffects.Shutdown();
    m_fontManager.Shutdown();
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
        m_screenEffects.Update(dt);

        m_stateManager.HandleInput(*this);
        if (!m_isRunning) break;

        m_stateManager.Update(*this, dt);
        if (!m_isRunning) break;

        // 1. Render game scene to offscreen texture
        BeginTextureMode(m_screenTexture);
        ClearBackground(Colors::BgDark);
        m_stateManager.Render(*this);
        EndTextureMode();

        // 2. Post-processing GPU shader pass to screen backbuffer
        BeginDrawing();
        ClearBackground(BLACK);
        m_screenEffects.RenderPostProcessing(m_screenTexture, WINDOW_WIDTH, WINDOW_HEIGHT);
        EndDrawing();
    }
}

PlayState* GameApp::GetActivePlayState() {
    return dynamic_cast<PlayState*>(m_stateManager.GetCurrentState());
}

const Inventory& GameApp::GetPlayStateInventory() const {
    PlayState* ps = const_cast<GameApp*>(this)->GetActivePlayState();
    if (ps) {
        return ps->GetRunManager().GetInventory();
    }
    static Inventory emptyInv;
    return emptyInv;
}

Inventory* GameApp::GetPlayStateInventoryMut() {
    PlayState* ps = GetActivePlayState();
    if (ps) {
        return &ps->GetRunManager().GetInventory();
    }
    return nullptr;
}

void GameApp::ApplyDraftCard(const Card& card) {
    EventCardAcquired evt{ card.id };
    m_eventBus.Publish(evt);
}

bool GameApp::UseRerollToken() {
    Inventory* inv = GetPlayStateInventoryMut();
    if (inv && inv->GetRerolls() > 0) {
        inv->SetRerolls(inv->GetRerolls() - 1);
        return true;
    }
    return false;
}

int GameApp::GetRemainingRerolls() const {
    const Inventory& inv = GetPlayStateInventory();
    return inv.GetRerolls();
}

} // namespace TetroShift
