#pragma once
#include "MusicTrack.hpp"
#include <raylib.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace TetroShift {

class MusicManager {
public:
    MusicManager();
    ~MusicManager();

    // Non-copyable
    MusicManager(const MusicManager&) = delete;
    MusicManager& operator=(const MusicManager&) = delete;

    void Initialize();
    void Shutdown();

    void PlayTrack(TrackId id, bool crossfade = true);
    void StopTrack(bool fadeOut = true);
    void Update(float dt);

    // Dynamic gameplay reactivity
    void SetUrgencyFactor(float urgency) noexcept; // 0.0f (safe) to 1.0f (danger)

    // Volume & Audio control
    void SetVolume(float volume) noexcept;
    [[nodiscard]] float GetVolume() const noexcept { return m_musicVolume; }
    void SetMuted(bool muted) noexcept;
    [[nodiscard]] bool IsMuted() const noexcept { return m_isMuted; }

    // HUD "Now Playing" Banner state
    [[nodiscard]] bool IsNowPlayingVisible() const noexcept { return m_nowPlayingTimer > 0.0f; }
    [[nodiscard]] float GetNowPlayingAlpha() const noexcept;
    [[nodiscard]] const std::string& GetNowPlayingTitle() const noexcept { return m_currentTrackTitle; }
    [[nodiscard]] const std::string& GetNowPlayingGenre() const noexcept { return m_currentTrackGenre; }
    [[nodiscard]] TrackId GetCurrentTrackId() const noexcept { return m_currentTrackId; }

    // Fixed Soundtrack Preference
    void SetFixedTrackIndex(int index) noexcept;
    [[nodiscard]] int GetFixedTrackIndex() const noexcept { return m_fixedTrackIndex; }
    [[nodiscard]] TrackId GetFixedTrackId() const noexcept;
    [[nodiscard]] const std::vector<TrackMetadata>& GetTrackCatalog() const noexcept { return m_catalog; }

private:
    struct LoadedTrack {
        TrackMetadata metadata;
        Music raylibMusic{};
        Sound proceduralSound{};
        bool isStream = false;
        bool isProcedural = false;
        bool isLoaded = false;
    };

    void LoadTrack(const TrackMetadata& meta);
    void GenerateProceduralTrack(TrackId id, LoadedTrack& track);

    bool m_initialized = false;
    bool m_isMuted = false;
    float m_musicVolume = 0.75f;
    float m_targetUrgency = 0.0f;
    float m_currentPitch = 1.0f;

    // Active track & Crossfading
    TrackId m_currentTrackId = TrackId::None;
    TrackId m_targetTrackId = TrackId::None;
    float m_currentTrackVolume = 0.0f;
    float m_targetTrackVolume = 0.0f;
    float m_crossfadeTimer = 0.0f;
    float m_crossfadeDuration = 1.2f;

    // Banner display state
    float m_nowPlayingTimer = 0.0f;
    const float NOW_PLAYING_DURATION = 4.0f;
    std::string m_currentTrackTitle;
    std::string m_currentTrackGenre;

    int m_fixedTrackIndex = 0; // 0 = Dynamic / By Floor, 1..8 = Fixed track
    std::vector<TrackMetadata> m_catalog;
    std::unordered_map<TrackId, LoadedTrack> m_tracks;
};

} // namespace TetroShift
