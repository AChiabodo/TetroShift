#pragma once
#include <raylib.h>
#include <vector>

namespace TetroShift {

class SoundSynth {
public:
    SoundSynth();
    ~SoundSynth();

    // Non-copyable
    SoundSynth(const SoundSynth&) = delete;
    SoundSynth& operator=(const SoundSynth&) = delete;

    void Initialize();
    void Shutdown();

    void PlayMove();
    void PlayRotate();
    void PlayDrop();
    void PlayLock();
    void PlayLineClear(int linesCount);
    void PlayTetris();
    void PlayCardSelect();
    void PlayGameOver();
    void PlayLevelUp();
    void PlayMenuHover();
    void PlayMenuBack();
    void PlayMenuToggle();

    void SetMuted(bool muted) noexcept;
    [[nodiscard]] bool IsMuted() const noexcept { return m_isMuted; }

    void SetMasterVolume(float volume) noexcept;
    [[nodiscard]] float GetMasterVolume() const noexcept { return m_masterVolume; }

private:
    [[nodiscard]] Sound GenerateTone(float freqStart, float freqEnd, float durationSeconds, float volume = 0.5f, int waveType = 0);
    [[nodiscard]] Sound GenerateNoise(float durationSeconds, float volume = 0.5f);

    bool m_initialized = false;
    bool m_isMuted = false;
    float m_masterVolume = 0.8f;

    Sound m_sndMove{};
    Sound m_sndRotate{};
    Sound m_sndDrop{};
    Sound m_sndLock{};
    Sound m_sndLineClear1{};
    Sound m_sndLineClear2{};
    Sound m_sndLineClear3{};
    Sound m_sndTetris{};
    Sound m_sndCardSelect{};
    Sound m_sndGameOver{};
    Sound m_sndLevelUp{};
    Sound m_sndMenuHover{};
    Sound m_sndMenuBack{};
    Sound m_sndMenuToggle{};
};

} // namespace TetroShift
