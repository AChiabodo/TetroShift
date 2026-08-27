#pragma once
#include "EventBus.hpp"
#include "states/GameStateManager.hpp"
#include "audio/SoundSynth.hpp"
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
    [[nodiscard]] CardDatabase& GetCardDatabase() noexcept { return m_cardDatabase; }

    // Inter-state bridge helpers
    [[nodiscard]] const Inventory& GetPlayStateInventory() const;
    void ApplyDraftCard(const struct Card& card);
    bool UseRerollToken();
    [[nodiscard]] int GetRemainingRerolls() const;

private:
    EventBus m_eventBus;
    GameStateManager m_stateManager;
    SoundSynth m_soundSynth;
    CardDatabase m_cardDatabase;
    bool m_isRunning = false;
};

} // namespace TetroShift
