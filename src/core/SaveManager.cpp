#include "SaveManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <ctime>
#include <iomanip>

namespace TetroShift {

namespace {

// Lightweight JSON parsing helpers
std::string ExtractStringField(const std::string& json, const std::string& key, const std::string& defVal = "") {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return defVal;

    size_t colon = json.find(':', pos + needle.length());
    if (colon == std::string::npos) return defVal;

    size_t firstQuote = json.find('\"', colon + 1);
    if (firstQuote == std::string::npos) return defVal;

    size_t secondQuote = json.find('\"', firstQuote + 1);
    if (secondQuote == std::string::npos) return defVal;

    return json.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

int ExtractIntField(const std::string& json, const std::string& key, int defVal = 0) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return defVal;

    size_t colon = json.find(':', pos + needle.length());
    if (colon == std::string::npos) return defVal;

    size_t start = colon + 1;
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\n' || json[start] == '\r')) {
        start++;
    }

    size_t end = start;
    if (end < json.length() && (json[end] == '-' || json[end] == '+')) end++;
    while (end < json.length() && (isdigit(json[end]) || json[end] == '.')) {
        end++;
    }

    if (start >= end) return defVal;
    try {
        return std::stoi(json.substr(start, end - start));
    } catch (...) {
        return defVal;
    }
}

float ExtractFloatField(const std::string& json, const std::string& key, float defVal = 0.0f) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return defVal;

    size_t colon = json.find(':', pos + needle.length());
    if (colon == std::string::npos) return defVal;

    size_t start = colon + 1;
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\n' || json[start] == '\r')) {
        start++;
    }

    size_t end = start;
    if (end < json.length() && (json[end] == '-' || json[end] == '+')) end++;
    while (end < json.length() && (isdigit(json[end]) || json[end] == '.')) {
        end++;
    }

    if (start >= end) return defVal;
    try {
        return std::stof(json.substr(start, end - start));
    } catch (...) {
        return defVal;
    }
}

bool ExtractBoolField(const std::string& json, const std::string& key, bool defVal = false) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return defVal;

    size_t colon = json.find(':', pos + needle.length());
    if (colon == std::string::npos) return defVal;

    size_t t = json.find("true", colon);
    size_t f = json.find("false", colon);
    size_t nextComma = json.find(',', colon);
    size_t nextBrace = json.find('}', colon);
    size_t limit = std::min(nextComma, nextBrace);

    if (t != std::string::npos && t < limit) return true;
    if (f != std::string::npos && f < limit) return false;
    return defVal;
}

std::vector<std::string> ExtractStringArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return result;

    size_t openBracket = json.find('[', pos + needle.length());
    if (openBracket == std::string::npos) return result;

    size_t closeBracket = json.find(']', openBracket + 1);
    if (closeBracket == std::string::npos) return result;

    size_t curr = openBracket + 1;
    while (curr < closeBracket) {
        size_t firstQuote = json.find('\"', curr);
        if (firstQuote == std::string::npos || firstQuote >= closeBracket) break;

        size_t secondQuote = json.find('\"', firstQuote + 1);
        if (secondQuote == std::string::npos || secondQuote > closeBracket) break;

        result.push_back(json.substr(firstQuote + 1, secondQuote - firstQuote - 1));
        curr = secondQuote + 1;
    }

    return result;
}

std::string GetCurrentTimestampStr() {
    auto now = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32) || defined(_MSC_VER)
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return oss.str();
}

} // anonymous namespace

SaveManager::SaveManager() = default;

SaveManager::~SaveManager() = default;

void SaveManager::EnsureSaveDirectory() {
    std::error_code ec;
    std::filesystem::create_directories("saves", ec);
}

std::string SaveManager::GetSlotFilename(int slotId) const {
    return "saves/slot_0" + std::to_string(slotId) + ".json";
}

void SaveManager::Initialize() {
    if (m_initialized) return;

    EnsureSaveDirectory();

    // 1. Try to load profile, or create default
    std::vector<ShopItemData> dummyShop;
    if (!LoadProfile(m_profile, m_settings, dummyShop, m_equippedTheme)) {
        // Initialize default profile
        m_profile.pilotName = "CYBER_PILOT_01";
        m_profile.pilotCallsign = "MORPHO-PRIME";
        m_profile.rankTitle = "NOVICE ARCHITECT";
        m_profile.level = 1;
        m_profile.currentExp = 0;
        m_profile.maxExp = 1000;
        m_profile.totalRuns = 0;
        m_profile.totalVictories = 0;
        m_profile.totalLinesCleared = 0;
        m_profile.highestScore = 0;
        m_profile.highestFloor = 1;
        m_profile.energyCredits = 250;

        SaveProfile(m_profile, m_settings, dummyShop, m_equippedTheme);
    }

    // 2. Try to load highscores, or create default list
    m_highScores = LoadHighScores();
    if (m_highScores.empty()) {
        m_highScores = {
            { 1, "WARP_RUNNER", 148200, 12, 114, "2026-08-30", "GRAND MASTER", { 255, 215, 0, 255 } },
            { 2, "NEO_ARCHITECT", 98500, 8, 72, "2026-08-29", "WARP MASTER", { 0, 240, 255, 255 } },
            { 3, "CYBER_PILOT", 42000, 4, 35, "2026-08-28", "ELITE RUNNER", { 190, 80, 255, 255 } }
        };
        SaveHighScores(m_highScores);
    }

    m_initialized = true;
}

void SaveManager::SaveProfile(
    const PlayerProfileData& profile,
    const GameSettings& settings,
    const std::vector<ShopItemData>& shopItems,
    const std::string& equippedTheme
) {
    EnsureSaveDirectory();
    m_profile = profile;
    m_settings = settings;
    m_equippedTheme = equippedTheme;

    std::ofstream file("saves/profile.json");
    if (!file.is_open()) return;

    file << "{\n";
    file << "  \"pilotName\": \"" << profile.pilotName << "\",\n";
    file << "  \"pilotCallsign\": \"" << profile.pilotCallsign << "\",\n";
    file << "  \"rankTitle\": \"" << profile.rankTitle << "\",\n";
    file << "  \"level\": " << profile.level << ",\n";
    file << "  \"currentExp\": " << profile.currentExp << ",\n";
    file << "  \"maxExp\": " << profile.maxExp << ",\n";
    file << "  \"totalRuns\": " << profile.totalRuns << ",\n";
    file << "  \"totalVictories\": " << profile.totalVictories << ",\n";
    file << "  \"totalLinesCleared\": " << profile.totalLinesCleared << ",\n";
    file << "  \"highestScore\": " << profile.highestScore << ",\n";
    file << "  \"highestFloor\": " << profile.highestFloor << ",\n";
    file << "  \"energyCredits\": " << profile.energyCredits << ",\n";
    file << "  \"equippedTheme\": \"" << equippedTheme << "\",\n";

    // Unlocked shop items
    file << "  \"unlockedShopItems\": [\n";
    bool firstItem = true;
    for (const auto& item : shopItems) {
        if (item.isUnlocked) {
            if (!firstItem) file << ",\n";
            file << "    \"" << item.id << "\"";
            firstItem = false;
        }
    }
    file << "\n  ],\n";

    // Settings
    file << "  \"settings\": {\n";
    file << "    \"masterVolume\": " << settings.masterVolume << ",\n";
    file << "    \"musicVolume\": " << settings.musicVolume << ",\n";
    file << "    \"sfxVolume\": " << settings.sfxVolume << ",\n";
    file << "    \"fixedSoundtrack\": " << settings.fixedSoundtrack << ",\n";
    file << "    \"screenShakeLevel\": " << settings.screenShakeLevel << ",\n";
    file << "    \"crtScanlines\": " << (settings.crtScanlines ? "true" : "false") << ",\n";
    file << "    \"softBodyWobble\": " << settings.softBodyWobble << ",\n";
    file << "    \"fastDAS\": " << (settings.fastDAS ? "true" : "false") << "\n";
    file << "  }\n";
    file << "}\n";
}

bool SaveManager::LoadProfile(
    PlayerProfileData& outProfile,
    GameSettings& outSettings,
    std::vector<ShopItemData>& shopItems,
    std::string& outEquippedTheme
) {
    std::ifstream file("saves/profile.json");
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();

    outProfile.pilotName = ExtractStringField(json, "pilotName", "CYBER_PILOT_01");
    outProfile.pilotCallsign = ExtractStringField(json, "pilotCallsign", "MORPHO-PRIME");
    outProfile.rankTitle = ExtractStringField(json, "rankTitle", "MASTER ARCHITECT III");
    outProfile.level = ExtractIntField(json, "level", 1);
    outProfile.currentExp = ExtractIntField(json, "currentExp", 0);
    outProfile.maxExp = ExtractIntField(json, "maxExp", 1000);
    outProfile.totalRuns = ExtractIntField(json, "totalRuns", 0);
    outProfile.totalVictories = ExtractIntField(json, "totalVictories", 0);
    outProfile.totalLinesCleared = ExtractIntField(json, "totalLinesCleared", 0);
    outProfile.highestScore = ExtractIntField(json, "highestScore", 0);
    outProfile.highestFloor = ExtractIntField(json, "highestFloor", 1);
    outProfile.energyCredits = ExtractIntField(json, "energyCredits", 250);

    outEquippedTheme = ExtractStringField(json, "equippedTheme", "Cyber Neon");

    // Settings
    outSettings.masterVolume = ExtractFloatField(json, "masterVolume", 0.80f);
    outSettings.musicVolume = ExtractFloatField(json, "musicVolume", 0.75f);
    outSettings.sfxVolume = ExtractFloatField(json, "sfxVolume", 0.85f);
    outSettings.fixedSoundtrack = ExtractIntField(json, "fixedSoundtrack", 0);
    outSettings.screenShakeLevel = ExtractIntField(json, "screenShakeLevel", 2);
    outSettings.crtScanlines = ExtractBoolField(json, "crtScanlines", true);
    outSettings.softBodyWobble = ExtractIntField(json, "softBodyWobble", 1);
    outSettings.fastDAS = ExtractBoolField(json, "fastDAS", false);

    // Unlocked shop items
    auto unlockedIds = ExtractStringArray(json, "unlockedShopItems");
    for (auto& item : shopItems) {
        for (const auto& uid : unlockedIds) {
            if (item.id == uid) {
                item.isUnlocked = true;
                break;
            }
        }
    }

    m_profile = outProfile;
    m_settings = outSettings;
    m_equippedTheme = outEquippedTheme;
    return true;
}

void SaveManager::AwardRunResults(int score, int floor, int lines, bool isVictory) {
    m_profile.totalRuns++;
    if (isVictory) m_profile.totalVictories++;
    m_profile.totalLinesCleared += lines;
    if (score > m_profile.highestScore) m_profile.highestScore = score;
    if (floor > m_profile.highestFloor) m_profile.highestFloor = floor;

    // Credits calculation: floor bonus + score bonus + lines
    int creditsEarned = (floor * 60) + (score / 800) + (lines * 2);
    if (isVictory) creditsEarned += 500;
    m_profile.energyCredits += creditsEarned;

    // XP calculation & Level Up Progression
    int xpEarned = (floor * 150) + (score / 400) + (lines * 10);
    if (isVictory) xpEarned += 1000;
    m_profile.currentExp += xpEarned;

    while (m_profile.currentExp >= m_profile.maxExp) {
        m_profile.currentExp -= m_profile.maxExp;
        m_profile.level++;
        m_profile.maxExp = static_cast<int>(static_cast<float>(m_profile.maxExp) * 1.25f);
    }

    // Update rank title based on level
    if (m_profile.level >= 20) m_profile.rankTitle = "GRAND ARCHITECT I";
    else if (m_profile.level >= 15) m_profile.rankTitle = "MASTER ARCHITECT III";
    else if (m_profile.level >= 10) m_profile.rankTitle = "ELITE RUNNER II";
    else if (m_profile.level >= 5)  m_profile.rankTitle = "QUANTUM PILOT";
    else m_profile.rankTitle = "NOVICE ARCHITECT";

    std::vector<ShopItemData> dummy;
    SaveProfile(m_profile, m_settings, dummy, m_equippedTheme);
}

void SaveManager::SaveHighScores(const std::vector<HighScoreEntry>& scores) {
    EnsureSaveDirectory();
    m_highScores = scores;

    std::ofstream file("saves/highscores.json");
    if (!file.is_open()) return;

    file << "[\n";
    for (size_t i = 0; i < scores.size(); ++i) {
        const auto& s = scores[i];
        file << "  {\n";
        file << "    \"rank\": " << (i + 1) << ",\n";
        file << "    \"pilotName\": \"" << s.pilotName << "\",\n";
        file << "    \"score\": " << s.score << ",\n";
        file << "    \"floor\": " << s.floorReached << ",\n";
        file << "    \"lines\": " << s.linesCleared << ",\n";
        file << "    \"date\": \"" << s.date << "\",\n";
        file << "    \"badge\": \"" << s.badge << "\"\n";
        file << "  }" << (i + 1 < scores.size() ? ",\n" : "\n");
    }
    file << "]\n";
}

std::vector<HighScoreEntry> SaveManager::LoadHighScores() {
    std::vector<HighScoreEntry> list;
    std::ifstream file("saves/highscores.json");
    if (!file.is_open()) return list;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Parse array of objects
    size_t pos = 0;
    while (true) {
        size_t objStart = content.find('{', pos);
        if (objStart == std::string::npos) break;
        size_t objEnd = content.find('}', objStart + 1);
        if (objEnd == std::string::npos) break;

        std::string block = content.substr(objStart, objEnd - objStart + 1);
        HighScoreEntry entry;
        entry.rank = ExtractIntField(block, "rank", static_cast<int>(list.size() + 1));
        entry.pilotName = ExtractStringField(block, "pilotName", "PILOT");
        entry.score = ExtractIntField(block, "score", 0);
        entry.floorReached = ExtractIntField(block, "floor", 1);
        entry.linesCleared = ExtractIntField(block, "lines", 0);
        entry.date = ExtractStringField(block, "date", "2026-08-31");
        entry.badge = ExtractStringField(block, "badge", "RUNNER");

        if (entry.rank == 1) entry.badgeColor = { 255, 215, 0, 255 }; // Gold
        else if (entry.rank == 2) entry.badgeColor = { 0, 240, 255, 255 }; // Cyan
        else if (entry.rank == 3) entry.badgeColor = { 190, 80, 255, 255 }; // Purple
        else entry.badgeColor = { 140, 150, 170, 255 }; // Silver/Dim

        list.push_back(entry);
        pos = objEnd + 1;
    }

    return list;
}

void SaveManager::AddHighScoreEntry(const HighScoreEntry& entry) {
    auto scores = LoadHighScores();
    scores.push_back(entry);

    // Sort by score descending
    std::sort(scores.begin(), scores.end(), [](const HighScoreEntry& a, const HighScoreEntry& b) {
        return a.score > b.score;
    });

    if (scores.size() > 10) {
        scores.resize(10);
    }

    for (size_t i = 0; i < scores.size(); ++i) {
        scores[i].rank = static_cast<int>(i + 1);
        if (i == 0) scores[i].badge = "GRAND MASTER";
        else if (i == 1) scores[i].badge = "WARP MASTER";
        else if (i == 2) scores[i].badge = "ELITE RUNNER";
        else scores[i].badge = "PILOT VETERAN";
    }

    SaveHighScores(scores);
}

void SaveManager::SaveRunSlot(int slotId, const SavedRunState& runState) {
    EnsureSaveDirectory();
    std::string path = GetSlotFilename(slotId);
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "{\n";
    file << "  \"slotId\": " << slotId << ",\n";
    file << "  \"state\": \"ActiveRun\",\n";
    file << "  \"runMode\": \"" << runState.runMode << "\",\n";
    file << "  \"timestamp\": \"" << GetCurrentTimestampStr() << "\",\n";
    file << "  \"floor\": " << runState.floor << ",\n";
    file << "  \"score\": " << runState.score << ",\n";
    file << "  \"linesTotal\": " << runState.linesTotal << ",\n";
    file << "  \"linesThisFloor\": " << runState.linesThisFloor << ",\n";
    file << "  \"floorLineTarget\": " << runState.floorLineTarget << ",\n";
    file << "  \"coins\": " << runState.coins << ",\n";
    file << "  \"rerollTokens\": " << runState.rerollTokens << ",\n";
    file << "  \"scoreMultiplier\": " << runState.scoreMultiplier << ",\n";
    file << "  \"speedMultiplier\": " << runState.speedMultiplier << ",\n";
    file << "  \"globalElasticity\": " << runState.globalElasticity << ",\n";
    file << "  \"holdPiece\": " << static_cast<int>(runState.holdPiece) << ",\n";
    file << "  \"canHold\": " << (runState.canHold ? "true" : "false") << ",\n";
    file << "  \"rngState\": " << runState.rngState << ",\n";

    // Cards list
    file << "  \"cards\": [\n";
    for (size_t i = 0; i < runState.cardIds.size(); ++i) {
        file << "    \"" << runState.cardIds[i] << "\"" << (i + 1 < runState.cardIds.size() ? ",\n" : "\n");
    }
    file << "  ],\n";

    // Grid rows
    file << "  \"grid\": [\n";
    for (size_t i = 0; i < runState.gridCells.size(); ++i) {
        file << "    \"" << runState.gridCells[i] << "\"" << (i + 1 < runState.gridCells.size() ? ",\n" : "\n");
    }
    file << "  ]\n";
    file << "}\n";
}

bool SaveManager::LoadRunSlot(int slotId, SavedRunState& outRunState) {
    std::string path = GetSlotFilename(slotId);
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();

    outRunState.slotId = slotId;
    outRunState.state = SaveSlotState::ActiveRun;
    outRunState.runMode = ExtractStringField(json, "runMode", "ROGUELIKE RUN");
    outRunState.timestamp = ExtractStringField(json, "timestamp", "2026-08-31 11:00");
    outRunState.floor = ExtractIntField(json, "floor", 1);
    outRunState.score = ExtractIntField(json, "score", 0);
    outRunState.linesTotal = ExtractIntField(json, "linesTotal", 0);
    outRunState.linesThisFloor = ExtractIntField(json, "linesThisFloor", 0);
    outRunState.floorLineTarget = ExtractIntField(json, "floorLineTarget", 6);
    outRunState.coins = ExtractIntField(json, "coins", 20);
    outRunState.rerollTokens = ExtractIntField(json, "rerollTokens", 1);
    outRunState.scoreMultiplier = ExtractFloatField(json, "scoreMultiplier", 1.0f);
    outRunState.speedMultiplier = ExtractFloatField(json, "speedMultiplier", 1.0f);
    outRunState.globalElasticity = ExtractFloatField(json, "globalElasticity", 1.0f);
    outRunState.holdPiece = static_cast<TetrominoType>(ExtractIntField(json, "holdPiece", 0));
    outRunState.canHold = ExtractBoolField(json, "canHold", true);
    outRunState.rngState = static_cast<uint32_t>(ExtractIntField(json, "rngState", 1337));

    outRunState.cardIds = ExtractStringArray(json, "cards");
    outRunState.gridCells = ExtractStringArray(json, "grid");

    return true;
}

void SaveManager::DeleteRunSlot(int slotId) {
    std::string path = GetSlotFilename(slotId);
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

std::vector<SaveSlotData> SaveManager::GetSaveSlotHeaders() {
    std::vector<SaveSlotData> slots;
    slots.resize(3);

    for (int i = 1; i <= 3; ++i) {
        SaveSlotData data;
        data.slotId = i;
        SavedRunState state;
        if (LoadRunSlot(i, state)) {
            data.state = SaveSlotState::ActiveRun;
            data.runMode = state.runMode + " // SECTOR 0" + std::to_string(state.floor);
            data.currentFloor = state.floor;
            data.currentScore = state.score;
            data.currentLines = state.linesTotal;
            data.energyCoins = state.coins;
            data.timestamp = state.timestamp;
            data.relicCards = state.cardIds;
            data.accentColor = (i == 1) ? Color{ 0, 240, 255, 255 } : (i == 2 ? Color{ 255, 215, 0, 255 } : Color{ 190, 80, 255, 255 });
        } else {
            data.state = SaveSlotState::Empty;
            data.runMode = "EMPTY MEMORY MATRIX";
            data.timestamp = "-- / -- / ----";
            data.accentColor = { 100, 110, 130, 255 };
        }
        slots[i - 1] = data;
    }

    return slots;
}

} // namespace TetroShift
