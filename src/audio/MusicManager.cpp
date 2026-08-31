#include "MusicManager.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace TetroShift {

MusicManager::MusicManager() = default;

MusicManager::~MusicManager() {
    Shutdown();
}

void MusicManager::Initialize() {
    if (m_initialized) return;

    m_catalog = GetDefaultTrackCatalog();
    for (const auto& meta : m_catalog) {
        LoadTrack(meta);
    }

    m_initialized = true;
}

void MusicManager::Shutdown() {
    if (!m_initialized) return;

    for (auto& [id, track] : m_tracks) {
        if (track.isLoaded) {
            if (track.isStream) {
                UnloadMusicStream(track.raylibMusic);
            } else if (track.isProcedural) {
                UnloadSound(track.proceduralSound);
            }
            track.isLoaded = false;
        }
    }
    m_tracks.clear();
    m_currentTrackId = TrackId::None;
    m_targetTrackId = TrackId::None;
    m_initialized = false;
}

void MusicManager::LoadTrack(const TrackMetadata& meta) {
    LoadedTrack track;
    track.metadata = meta;

    // Check if physical file exists on disk
    if (FileExists(meta.filename.c_str())) {
        track.raylibMusic = LoadMusicStream(meta.filename.c_str());
        if (track.raylibMusic.stream.buffer != nullptr) {
            track.raylibMusic.looping = true;
            track.isStream = true;
            track.isLoaded = true;
        }
    }

    // If file is not present or failed to load, synthesize in-memory procedural fallback
    if (!track.isLoaded) {
        GenerateProceduralTrack(meta.id, track);
    }

    m_tracks[meta.id] = track;
}

void MusicManager::GenerateProceduralTrack(TrackId id, LoadedTrack& track) {
    const int sampleRate = 22050; // High quality and memory efficient
    float durationSeconds = 12.0f; // 12-second seamless procedural loop
    float bpm = track.metadata.baseBpm;
    if (bpm <= 0.0f) bpm = 120.0f;

    const int totalFrames = static_cast<int>(static_cast<float>(sampleRate) * durationSeconds);
    short* buffer = static_cast<short*>(MemAlloc(totalFrames * sizeof(short)));
    if (!buffer) return;

    // Select harmonic key and base frequency based on track type
    float rootFreq = 220.0f; // A3 default
    switch (id) {
        case TrackId::MenuTheme:       rootFreq = 220.00f; break; // A3 (Am)
        case TrackId::EarlyFloorTheme: rootFreq = 146.83f; break; // D3 (Dm)
        case TrackId::MidFloorTheme:   rootFreq = 185.00f; break; // F#3 (F#m)
        case TrackId::HighFloorTheme:  rootFreq = 130.81f; break; // C3 (Cm)
        case TrackId::BossFloorTheme:  rootFreq = 164.81f; break; // E3 (Em)
        case TrackId::DraftTheme:      rootFreq = 196.00f; break; // G3 (Gm)
        case TrackId::GameOverTheme:   rootFreq = 220.00f; break; // A3 (Am)
        case TrackId::EndlessTheme:    rootFreq = 246.94f; break; // B3 (Bm)
        default: break;
    }

    const float beatDuration = 60.0f / bpm;
    const float sixteenthDuration = beatDuration * 0.25f;

    // Minor pentatonic / natural minor scale intervals (semitones)
    const int scaleOffsets[7] = { 0, 3, 5, 7, 10, 12, 15 };

    float bassPhase = 0.0f;
    float arpPhase = 0.0f;
    float padPhase1 = 0.0f;
    float padPhase2 = 0.0f;

    for (int i = 0; i < totalFrames; ++i) {
        float timeSec = static_cast<float>(i) / static_cast<float>(sampleRate);
        int current16th = static_cast<int>(timeSec / sixteenthDuration);
        int currentBeat = static_cast<int>(timeSec / beatDuration);

        // 1. Bassline (Saw / Triangle with 8th-note pump)
        int bassNoteIndex = (currentBeat % 4 == 3) ? 2 : (currentBeat % 4 == 2 ? 1 : 0);
        float bassFreq = rootFreq * 0.5f * std::pow(2.0f, static_cast<float>(scaleOffsets[bassNoteIndex]) / 12.0f);
        bassPhase += (2.0f * 3.14159265f * bassFreq) / static_cast<float>(sampleRate);
        float bassSample = (std::sin(bassPhase) >= 0.0f) ? 0.6f : -0.6f;
        bassSample += std::sin(bassPhase * 0.5f) * 0.4f; // Sub-bass reinforcement
        float bassEnv = 1.0f - std::fmod(timeSec, beatDuration * 0.5f) / (beatDuration * 0.5f);

        // 2. Arpeggio / Lead synth (Crisp 16th-note sweep)
        int arpStep = current16th % 8;
        int noteOffset = scaleOffsets[arpStep % 6];
        if (arpStep >= 4) noteOffset += 12; // Octave lift
        float arpFreq = rootFreq * std::pow(2.0f, static_cast<float>(noteOffset) / 12.0f);
        arpPhase += (2.0f * 3.14159265f * arpFreq) / static_cast<float>(sampleRate);
        float arpSample = 2.0f * std::abs(2.0f * (arpPhase / (2.0f * 3.14159265f) - std::floor(arpPhase / (2.0f * 3.14159265f) + 0.5f))) - 1.0f;
        float arpEnv = 1.0f - std::fmod(timeSec, sixteenthDuration) / sixteenthDuration;

        // 3. Ambient Pad (Warm Detuned Sines)
        padPhase1 += (2.0f * 3.14159265f * rootFreq * 1.002f) / static_cast<float>(sampleRate);
        padPhase2 += (2.0f * 3.14159265f * rootFreq * 1.503f) / static_cast<float>(sampleRate);
        float padSample = (std::sin(padPhase1) + std::sin(padPhase2)) * 0.5f;

        // 4. Drums / Percussion
        float drumSample = 0.0f;
        // Four-on-the-floor Kick on beats
        float beatFrac = std::fmod(timeSec, beatDuration) / beatDuration;
        if (beatFrac < 0.15f) {
            float kickFreq = 140.0f * (1.0f - (beatFrac / 0.15f)) + 45.0f;
            float kickPhase = 2.0f * 3.14159265f * kickFreq * beatFrac;
            drumSample += std::sin(kickPhase) * (1.0f - beatFrac / 0.15f) * 0.85f;
        }

        // Snare / Clap on beats 2 and 4
        if (currentBeat % 2 == 1 && beatFrac < 0.18f) {
            float noise = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
            drumSample += noise * (1.0f - beatFrac / 0.18f) * 0.45f;
        }

        // Mix all layers with balanced gains
        float mixed = (bassSample * bassEnv * 0.28f) +
                      (arpSample * arpEnv * 0.22f) +
                      (padSample * 0.18f) +
                      (drumSample * 0.32f);

        // Soft clipper limiter
        mixed = std::tanh(mixed);

        buffer[i] = static_cast<short>(mixed * 32767.0f);
    }

    Wave wave{};
    wave.frameCount = totalFrames;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = buffer;

    track.proceduralSound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    track.isProcedural = true;
    track.isLoaded = true;
}

void MusicManager::PlayTrack(TrackId id, bool crossfade) {
    if (!m_initialized || id == TrackId::None) return;
    if (id == m_currentTrackId && !m_tracks[id].isProcedural) return;

    auto it = m_tracks.find(id);
    if (it == m_tracks.end() || !it->second.isLoaded) return;

    // Trigger HUD "Now Playing" Banner
    m_currentTrackTitle = it->second.metadata.title;
    m_currentTrackGenre = it->second.metadata.genre;
    m_nowPlayingTimer = NOW_PLAYING_DURATION;

    if (!crossfade || m_currentTrackId == TrackId::None) {
        // Instant switch
        StopTrack(false);
        m_currentTrackId = id;
        m_targetTrackId = TrackId::None;
        m_currentTrackVolume = 1.0f;
        m_targetTrackVolume = 0.0f;

        auto& track = m_tracks[m_currentTrackId];
        if (track.isStream) {
            PlayMusicStream(track.raylibMusic);
            SetMusicVolume(track.raylibMusic, m_isMuted ? 0.0f : (m_musicVolume * track.metadata.defaultVolume));
        } else if (track.isProcedural) {
            PlaySound(track.proceduralSound);
            SetSoundVolume(track.proceduralSound, m_isMuted ? 0.0f : (m_musicVolume * track.metadata.defaultVolume));
        }
    } else {
        // Start crossfade to new target
        m_targetTrackId = id;
        m_targetTrackVolume = 0.0f;
        m_crossfadeTimer = m_crossfadeDuration;

        auto& targetTrack = m_tracks[m_targetTrackId];
        if (targetTrack.isStream) {
            PlayMusicStream(targetTrack.raylibMusic);
            SetMusicVolume(targetTrack.raylibMusic, 0.0f);
        } else if (targetTrack.isProcedural) {
            PlaySound(targetTrack.proceduralSound);
            SetSoundVolume(targetTrack.proceduralSound, 0.0f);
        }
    }
}

void MusicManager::StopTrack(bool fadeOut) {
    if (m_currentTrackId != TrackId::None) {
        auto it = m_tracks.find(m_currentTrackId);
        if (it != m_tracks.end() && it->second.isLoaded) {
            if (it->second.isStream) {
                StopMusicStream(it->second.raylibMusic);
            } else if (it->second.isProcedural) {
                StopSound(it->second.proceduralSound);
            }
        }
    }
    if (!fadeOut) {
        m_currentTrackId = TrackId::None;
        m_targetTrackId = TrackId::None;
    }
}

void MusicManager::SetUrgencyFactor(float urgency) noexcept {
    m_targetUrgency = std::min(1.0f, std::max(0.0f, urgency));
}

void MusicManager::SetVolume(float volume) noexcept {
    m_musicVolume = std::min(1.0f, std::max(0.0f, volume));
}

void MusicManager::SetMuted(bool muted) noexcept {
    m_isMuted = muted;
}

float MusicManager::GetNowPlayingAlpha() const noexcept {
    if (m_nowPlayingTimer <= 0.0f) return 0.0f;
    // Fade in during first 0.8s, hold, fade out during last 0.8s
    if (m_nowPlayingTimer > NOW_PLAYING_DURATION - 0.8f) {
        return (NOW_PLAYING_DURATION - m_nowPlayingTimer) / 0.8f;
    }
    if (m_nowPlayingTimer < 0.8f) {
        return m_nowPlayingTimer / 0.8f;
    }
    return 1.0f;
}

void MusicManager::Update(float dt) {
    if (!m_initialized) return;

    // Update banner timer
    if (m_nowPlayingTimer > 0.0f) {
        m_nowPlayingTimer -= dt;
        if (m_nowPlayingTimer < 0.0f) m_nowPlayingTimer = 0.0f;
    }

    // Lerp urgency pitch: baseline 1.0f -> up to 1.08f under high danger
    float targetPitch = 1.0f + (m_targetUrgency * 0.08f);
    m_currentPitch += (targetPitch - m_currentPitch) * dt * 2.0f;

    // Handle Crossfading
    if (m_targetTrackId != TrackId::None) {
        m_crossfadeTimer -= dt;
        float progress = 1.0f - (m_crossfadeTimer / m_crossfadeDuration);
        progress = std::min(1.0f, std::max(0.0f, progress));

        m_currentTrackVolume = 1.0f - progress;
        m_targetTrackVolume = progress;

        if (m_crossfadeTimer <= 0.0f) {
            // Crossfade complete: swap current with target
            if (m_currentTrackId != TrackId::None) {
                auto itOld = m_tracks.find(m_currentTrackId);
                if (itOld != m_tracks.end() && itOld->second.isLoaded) {
                    if (itOld->second.isStream) StopMusicStream(itOld->second.raylibMusic);
                    else if (itOld->second.isProcedural) StopSound(itOld->second.proceduralSound);
                }
            }
            m_currentTrackId = m_targetTrackId;
            m_targetTrackId = TrackId::None;
            m_currentTrackVolume = 1.0f;
            m_targetTrackVolume = 0.0f;
        }
    } else {
        m_currentTrackVolume = 1.0f;
    }

    // Update and apply volumes/pitch to current track
    if (m_currentTrackId != TrackId::None) {
        auto it = m_tracks.find(m_currentTrackId);
        if (it != m_tracks.end() && it->second.isLoaded) {
            float effVol = m_isMuted ? 0.0f : (m_musicVolume * it->second.metadata.defaultVolume * m_currentTrackVolume);
            if (it->second.isStream) {
                UpdateMusicStream(it->second.raylibMusic);
                SetMusicVolume(it->second.raylibMusic, effVol);
                SetMusicPitch(it->second.raylibMusic, m_currentPitch);
            } else if (it->second.isProcedural) {
                SetSoundVolume(it->second.proceduralSound, effVol);
                SetSoundPitch(it->second.proceduralSound, m_currentPitch);
                // Procedural sound loop restart if stopped
                if (!IsSoundPlaying(it->second.proceduralSound) && effVol > 0.001f) {
                    PlaySound(it->second.proceduralSound);
                }
            }
        }
    }

    // Update target track during crossfade
    if (m_targetTrackId != TrackId::None) {
        auto itTarget = m_tracks.find(m_targetTrackId);
        if (itTarget != m_tracks.end() && itTarget->second.isLoaded) {
            float effVol = m_isMuted ? 0.0f : (m_musicVolume * itTarget->second.metadata.defaultVolume * m_targetTrackVolume);
            if (itTarget->second.isStream) {
                UpdateMusicStream(itTarget->second.raylibMusic);
                SetMusicVolume(itTarget->second.raylibMusic, effVol);
                SetMusicPitch(itTarget->second.raylibMusic, m_currentPitch);
            } else if (itTarget->second.isProcedural) {
                SetSoundVolume(itTarget->second.proceduralSound, effVol);
                SetSoundPitch(itTarget->second.proceduralSound, m_currentPitch);
            }
        }
    }
}

void MusicManager::SetFixedTrackIndex(int index) noexcept {
    if (index < 0) index = 0;
    if (index > static_cast<int>(m_catalog.size())) index = static_cast<int>(m_catalog.size());
    m_fixedTrackIndex = index;
}

TrackId MusicManager::GetFixedTrackId() const noexcept {
    if (m_fixedTrackIndex >= 1 && m_fixedTrackIndex <= static_cast<int>(m_catalog.size())) {
        return m_catalog[m_fixedTrackIndex - 1].id;
    }
    return TrackId::None;
}

} // namespace TetroShift
