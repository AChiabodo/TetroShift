#pragma once
#include "EventBus.hpp"
#include "states/GameStateManager.hpp"
#include "audio/SoundSynth.hpp"
#include "audio/MusicManager.hpp"
#include "core/SaveManager.hpp"
#include "roguelike/CardDatabase.hpp"
#include "roguelike/Inventory.hpp"
#include <memory>

namespace TetroShift {

class GameApp {
public:
    GameApp();
    ~GameApp();

    // Non-copyable, non-movable
    GameApp(const GameApp&) = delete;
    GameApp& operator=(const GameApp&) = delete;

    void Run();

    [[nodiscard]] EventBus& GetEventBus() noexcept { return m_eventBus; }
    [[nodiscard]] GameStateManager& GetStateManager() noexcept { return m_stateManager; }
    [[nodiscard]] SoundSynth& GetSoundSynth() noexcept { return m_soundSynth; }
    [[nodiscard]] MusicManager& GetMusicManager() noexcept { return m_musicManager; }
    [[nodiscard]] SaveManager& GetSaveManager() noexcept { return m_saveManager; }
    [[nodiscard]] CardDatabase& GetCardDatabase() noexcept { return m_cardDatabase; }

    // Inter-state bridge helpers
    class PlayState* GetActivePlayState();
    [[nodiscard]] const Inventory& GetPlayStateInventory() const;
    Inventory* GetPlayStateInventoryMut();
    void ApplyDraftCard(const struct Card& card);
    bool UseRerollToken();
    [[nodiscard]] int GetRemainingRerolls() const;
    void RequestExit() noexcept { m_isRunning = false; }
    [[nodiscard]] bool IsRunning() const noexcept { return m_isRunning; }

private:
    EventBus m_eventBus;
    GameStateManager m_stateManager;
    SoundSynth m_soundSynth;
    MusicManager m_musicManager;
    SaveManager m_saveManager;
    CardDatabase m_cardDatabase;
    bool m_isRunning = false;
};

} // namespace TetroShift
