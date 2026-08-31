#pragma once
#include "FloorHazard.hpp"
#include <random>
#include <string>

namespace TetroShift {

class IGrid;
class ActivePiece;
class ScreenEffects;
class ParticleSystem;
class SoundSynth;

class HazardManager {
public:
    HazardManager();

    void Reset();
    void SetFloor(int floorNumber);

    void Update(float dt, IGrid& grid, ActivePiece& piece, ScreenEffects& effects, ParticleSystem& particles, SoundSynth& sound);

    [[nodiscard]] const FloorHazardConfig& GetConfig() const noexcept { return m_config; }
    [[nodiscard]] HazardType GetActiveHazardType() const noexcept { return m_config.type; }
    [[nodiscard]] bool HasActiveHazard() const noexcept { return m_config.type != HazardType::None; }
    [[nodiscard]] bool IsPulseActive() const noexcept { return m_isPulseActive; }
    [[nodiscard]] bool AreControlsInverted() const noexcept { return m_controlsInverted; }
    [[nodiscard]] float GetGravitySpeedMultiplier() const noexcept { return m_gravityMultiplier; }
    [[nodiscard]] float GetPulseTimer() const noexcept { return m_pulseTimer; }
    [[nodiscard]] float GetPulseInterval() const noexcept { return m_config.pulseInterval; }

    [[nodiscard]] std::string GetActiveStatusText() const;

private:
    FloorHazardConfig m_config;
    int m_currentFloor = 1;
    float m_pulseTimer = 0.0f;
    float m_pulseActiveDuration = 0.0f;
    bool m_isPulseActive = false;
    bool m_controlsInverted = false;
    float m_gravityMultiplier = 1.0f;
    std::mt19937 m_rng;
};

} // namespace TetroShift
