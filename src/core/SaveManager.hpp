#pragma once
#include "states/MenuTypes.hpp"
#include <string>
#include <vector>
#include <memory>

namespace TetroShift {

class SaveManager {
public:
    SaveManager();
    ~SaveManager();

    // Non-copyable
    SaveManager(const SaveManager&) = delete;
    SaveManager& operator=(const SaveManager&) = delete;

    void Initialize();

    // Profile & Meta-progression
    void SaveProfile(
        const PlayerProfileData& profile,
        const GameSettings& settings,
        const std::vector<ShopItemData>& shopItems,
        const std::string& equippedTheme
    );

    bool LoadProfile(
        PlayerProfileData& outProfile,
        GameSettings& outSettings,
        std::vector<ShopItemData>& shopItems,
        std::string& outEquippedTheme
    );

    void AwardRunResults(int score, int floor, int lines, bool isVictory);

    // High Scores
    void SaveHighScores(const std::vector<HighScoreEntry>& scores);
    [[nodiscard]] std::vector<HighScoreEntry> LoadHighScores();
    void AddHighScoreEntry(const HighScoreEntry& entry);

    // Daily seed helpers
    static uint32_t ComputeDailySeed();
    static std::string GetDailyDateString();

    // Roguelike Run Slots
    void SaveRunSlot(int slotId, const SavedRunState& runState);
    bool LoadRunSlot(int slotId, SavedRunState& outRunState);
    void DeleteRunSlot(int slotId);
    [[nodiscard]] std::vector<SaveSlotData> GetSaveSlotHeaders();

    [[nodiscard]] const PlayerProfileData& GetProfile() const noexcept { return m_profile; }
    [[nodiscard]] PlayerProfileData& GetProfile() noexcept { return m_profile; }
    [[nodiscard]] const GameSettings& GetSettings() const noexcept { return m_settings; }
    [[nodiscard]] GameSettings& GetSettings() noexcept { return m_settings; }

private:
    void EnsureSaveDirectory();
    std::string GetSlotFilename(int slotId) const;

    PlayerProfileData m_profile;
    GameSettings m_settings;
    std::string m_equippedTheme = "Cyber Neon";
    std::vector<HighScoreEntry> m_highScores;
    bool m_initialized = false;
};

} // namespace TetroShift
