#include "SoundSynth.hpp"
#include <cmath>
#include <cstdlib>

namespace TetroShift {

SoundSynth::SoundSynth() {
    // Will be initialized after Raylib InitAudioDevice()
}

SoundSynth::~SoundSynth() {
    Shutdown();
}

Sound SoundSynth::GenerateTone(float freqStart, float freqEnd, float durationSeconds, float volume, int waveType) {
    const int sampleRate = 44100;
    const int frameCount = static_cast<int>(static_cast<float>(sampleRate) * durationSeconds);
    short* buffer = static_cast<short*>(MemAlloc(frameCount * sizeof(short)));

    if (!buffer) {
        return Sound{};
    }

    float phase = 0.0f;
    for (int i = 0; i < frameCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(frameCount);
        const float currentFreq = freqStart + (freqEnd - freqStart) * t;
        const float phaseStep = (2.0f * 3.14159265f * currentFreq) / static_cast<float>(sampleRate);
        phase += phaseStep;

        // Envelope (quick attack, smooth decay)
        float envelope = 1.0f - t;
        if (t < 0.1f) {
            envelope = t / 0.1f;
        }

        float sample = 0.0f;
        if (waveType == 0) {
            // Sine
            sample = std::sin(phase);
        } else if (waveType == 1) {
            // Square
            sample = (std::sin(phase) >= 0.0f) ? 0.8f : -0.8f;
        } else {
            // Triangle
            sample = 2.0f * std::abs(2.0f * (phase / (2.0f * 3.14159265f) - std::floor(phase / (2.0f * 3.14159265f) + 0.5f))) - 1.0f;
        }

        buffer[i] = static_cast<short>(sample * envelope * volume * 32767.0f);
    }

    Wave wave{};
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = buffer;

    Sound snd = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return snd;
}

Sound SoundSynth::GenerateNoise(float durationSeconds, float volume) {
    const int sampleRate = 44100;
    const int frameCount = static_cast<int>(static_cast<float>(sampleRate) * durationSeconds);
    short* buffer = static_cast<short*>(MemAlloc(frameCount * sizeof(short)));

    if (!buffer) {
        return Sound{};
    }

    for (int i = 0; i < frameCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(frameCount);
        float envelope = (1.0f - t) * (1.0f - t);
        float randVal = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
        buffer[i] = static_cast<short>(randVal * envelope * volume * 32767.0f);
    }

    Wave wave{};
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = buffer;

    Sound snd = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return snd;
}

void SoundSynth::Initialize() {
    if (m_initialized) return;

    if (IsAudioDeviceReady()) {
        m_sndMove = GenerateTone(420.0f, 480.0f, 0.04f, 0.25f, 0);       // Quick blip
        m_sndRotate = GenerateTone(520.0f, 780.0f, 0.06f, 0.35f, 2);     // Crisp triangle chirp
        m_sndDrop = GenerateTone(220.0f, 80.0f, 0.12f, 0.5f, 1);         // Heavy thud
        m_sndLock = GenerateTone(350.0f, 260.0f, 0.05f, 0.3f, 0);        // Lock click
        m_sndLineClear1 = GenerateTone(587.33f, 880.0f, 0.18f, 0.45f, 0); // D5 -> A5
        m_sndLineClear2 = GenerateTone(659.25f, 987.77f, 0.22f, 0.5f, 0); // E5 -> B5
        m_sndLineClear3 = GenerateTone(783.99f, 1174.66f, 0.25f, 0.55f, 0);// G5 -> D6
        m_sndTetris = GenerateTone(523.25f, 1318.51f, 0.45f, 0.65f, 1);    // Triumphant square sweep
        m_sndCardSelect = GenerateTone(440.0f, 1200.0f, 0.3f, 0.5f, 2);    // Magical chime
        m_sndGameOver = GenerateTone(320.0f, 60.0f, 0.7f, 0.6f, 1);        // Descending doom
        m_sndLevelUp = GenerateTone(392.0f, 1046.50f, 0.35f, 0.55f, 0);    // G4 -> C6 victory
        m_sndMenuHover = GenerateTone(600.0f, 750.0f, 0.025f, 0.20f, 0);   // Soft cursor hover tick
        m_sndMenuBack = GenerateTone(450.0f, 220.0f, 0.12f, 0.35f, 2);     // Descending cancel chirp
        m_sndMenuToggle = GenerateTone(480.0f, 620.0f, 0.06f, 0.35f, 1);   // Crisp toggle click

        m_initialized = true;
        ::SetMasterVolume(m_masterVolume);
    }
}

void SoundSynth::Shutdown() {
    if (!m_initialized) return;

    UnloadSound(m_sndMove);
    UnloadSound(m_sndRotate);
    UnloadSound(m_sndDrop);
    UnloadSound(m_sndLock);
    UnloadSound(m_sndLineClear1);
    UnloadSound(m_sndLineClear2);
    UnloadSound(m_sndLineClear3);
    UnloadSound(m_sndTetris);
    UnloadSound(m_sndCardSelect);
    UnloadSound(m_sndGameOver);
    UnloadSound(m_sndLevelUp);
    UnloadSound(m_sndMenuHover);
    UnloadSound(m_sndMenuBack);
    UnloadSound(m_sndMenuToggle);

    m_initialized = false;
}

void SoundSynth::SetMuted(bool muted) noexcept {
    m_isMuted = muted;
    if (m_isMuted) {
        ::SetMasterVolume(0.0f);
    } else {
        ::SetMasterVolume(m_masterVolume);
    }
}

void SoundSynth::SetMasterVolume(float volume) noexcept {
    m_masterVolume = (volume < 0.0f) ? 0.0f : ((volume > 1.0f) ? 1.0f : volume);
    if (!m_isMuted) {
        ::SetMasterVolume(m_masterVolume);
    }
}

void SoundSynth::PlayMove() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndMove);
}

void SoundSynth::PlayRotate() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndRotate);
}

void SoundSynth::PlayDrop() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndDrop);
}

void SoundSynth::PlayLock() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndLock);
}

void SoundSynth::PlayLineClear(int linesCount) {
    if (!m_initialized || m_isMuted) return;
    if (linesCount >= 4) {
        PlaySound(m_sndTetris);
    } else if (linesCount == 3) {
        PlaySound(m_sndLineClear3);
    } else if (linesCount == 2) {
        PlaySound(m_sndLineClear2);
    } else {
        PlaySound(m_sndLineClear1);
    }
}

void SoundSynth::PlayTetris() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndTetris);
}

void SoundSynth::PlayCardSelect() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndCardSelect);
}

void SoundSynth::PlayGameOver() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndGameOver);
}

void SoundSynth::PlayLevelUp() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndLevelUp);
}

void SoundSynth::PlayMenuHover() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndMenuHover);
}

void SoundSynth::PlayMenuBack() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndMenuBack);
}

void SoundSynth::PlayMenuToggle() {
    if (m_initialized && !m_isMuted) PlaySound(m_sndMenuToggle);
}

} // namespace TetroShift
