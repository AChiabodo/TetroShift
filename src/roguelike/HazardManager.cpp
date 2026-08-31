#include "HazardManager.hpp"
#include "grid/IGrid.hpp"
#include "piece/ActivePiece.hpp"
#include "render/ScreenEffects.hpp"
#include "render/ParticleSystem.hpp"
#include "audio/SoundSynth.hpp"
#include <chrono>
#include <cmath>

namespace TetroShift {

HazardManager::HazardManager() {
    uint32_t seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
    m_rng.seed(seed);
    Reset();
}

void HazardManager::Reset() {
    m_config = FloorHazardConfig{};
    m_currentFloor = 1;
    m_pulseTimer = 0.0f;
    m_pulseActiveDuration = 0.0f;
    m_isPulseActive = false;
    m_controlsInverted = false;
    m_gravityMultiplier = 1.0f;
}

void HazardManager::SetFloor(int floorNumber) {
    m_currentFloor = floorNumber;
    m_pulseTimer = 0.0f;
    m_pulseActiveDuration = 0.0f;
    m_isPulseActive = false;
    m_controlsInverted = false;
    m_gravityMultiplier = 1.0f;

    switch (floorNumber) {
        case 2:
            m_config.type = HazardType::SolarFlare;
            m_config.title = "SOLAR FLARE FLUX";
            m_config.warningDesc = "High-energy solar wind: Fall speed +35% and Score +50%";
            m_config.themeColor = Colors::PieceBomb;
            m_config.pulseInterval = 8.0f;
            m_config.isBossSector = false;
            m_gravityMultiplier = 1.35f;
            break;

        case 5:
            m_config.type = HazardType::GravityFlux;
            m_config.title = "BOSS 01 // GRAVITY FLUX";
            m_config.warningDesc = "Unstable gravitational core: Periodic 2.5x speed spikes and drift waves";
            m_config.themeColor = Colors::PieceT;
            m_config.pulseInterval = 6.0f;
            m_config.isBossSector = true;
            break;

        case 7:
            m_config.type = HazardType::CryoChamber;
            m_config.title = "CRYO-SUBZERO MATRIX";
            m_config.warningDesc = "Subzero temperatures: Elastic gelatinous blocks with high inertia";
            m_config.themeColor = Colors::PieceJelly;
            m_config.pulseInterval = 7.0f;
            m_config.isBossSector = false;
            break;

        case 10:
            m_config.type = HazardType::SingularityCore;
            m_config.title = "BOSS 02 // SINGULARITY CORE";
            m_config.warningDesc = "Gravitational collapse: Periodic garbage row injections with titanium blocks";
            m_config.themeColor = Colors::PieceIron;
            m_config.pulseInterval = 7.5f;
            m_config.isBossSector = true;
            break;

        case 12:
            m_config.type = HazardType::MagneticStorm;
            m_config.title = "IONIC MAGNETIC STORM";
            m_config.warningDesc = "Polar magnetic interference shifting block trajectories";
            m_config.themeColor = Colors::PieceI;
            m_config.pulseInterval = 5.5f;
            m_config.isBossSector = false;
            break;

        case 15:
            m_config.type = HazardType::GlitchMatrix;
            m_config.title = "BOSS 03 // VOID GLITCH MATRIX";
            m_config.warningDesc = "Corrupted reality: Periodic command inversion and chromatic aberration";
            m_config.themeColor = RED;
            m_config.pulseInterval = 8.0f;
            m_config.isBossSector = true;
            break;

        default:
            if (floorNumber > 15) {
                // Endless hyper-flux
                if (floorNumber % 3 == 0) {
                    m_config.type = HazardType::GlitchMatrix;
                    m_config.title = "HYPER-VOID GLITCH";
                    m_config.warningDesc = "System integrity compromised: Inverted controls active!";
                    m_config.themeColor = RED;
                    m_config.pulseInterval = 7.0f;
                    m_config.isBossSector = true;
                } else {
                    m_config.type = HazardType::GravityFlux;
                    m_config.title = "HYPER-GRAVITY ANOMALY";
                    m_config.warningDesc = "Gravitational acceleration wave";
                    m_config.themeColor = Colors::PieceT;
                    m_config.pulseInterval = 5.0f;
                    m_config.isBossSector = false;
                }
            } else {
                m_config.type = HazardType::None;
                m_config.title = "NORMAL SECTOR";
                m_config.warningDesc = "Standard gravitational equilibrium";
                m_config.themeColor = Colors::TextWhite;
                m_config.pulseInterval = 10.0f;
                m_config.isBossSector = false;
            }
            break;
    }
}

void HazardManager::Update(float dt, IGrid& grid, ActivePiece& piece, ScreenEffects& effects, ParticleSystem& particles, SoundSynth& sound) {
    if (m_config.type == HazardType::None) {
        m_isPulseActive = false;
        m_controlsInverted = false;
        m_gravityMultiplier = 1.0f;
        return;
    }

    m_pulseTimer += dt;

    if (!m_isPulseActive) {
        // Charging phase
        if (m_pulseTimer >= m_config.pulseInterval) {
            m_pulseTimer = 0.0f;
            m_isPulseActive = true;
            m_pulseActiveDuration = 0.0f;

            // Trigger pulse start effects
            if (m_config.type == HazardType::GravityFlux) {
                m_gravityMultiplier = 2.5f;
                effects.AddTrauma(0.35f);
                sound.PlayDrop();
                particles.AddPopup("GRAVITY SURGE (2.5X)!", { PLAYFIELD_X + 160.0f, PLAYFIELD_Y + 140.0f }, Colors::PieceT, 1.4f);
            } else if (m_config.type == HazardType::SingularityCore) {
                effects.AddTrauma(0.45f);
                effects.TriggerFlash(Colors::PieceIron, 0.4f);
                sound.PlayRotate();

                // Inject garbage row at bottom
                std::uniform_int_distribution<int> colDist(0, grid.GetWidth() - 1);
                int holeCol = colDist(m_rng);
                grid.PushGarbageRow(holeCol, CellType::HeavyIron, Colors::PieceIron);
                particles.AddPopup("SINGULARITY GARBAGE ROW!", { PLAYFIELD_X + 160.0f, PLAYFIELD_Y + 200.0f }, Colors::PieceBomb, 1.4f);
            } else if (m_config.type == HazardType::GlitchMatrix) {
                m_controlsInverted = true;
                effects.AddTrauma(0.40f);
                effects.TriggerFlash(RED, 0.5f);
                sound.PlayRotate();
                particles.AddPopup("⚠ CONTROLS INVERTED!", { PLAYFIELD_X + 160.0f, PLAYFIELD_Y + 120.0f }, RED, 1.6f);
            } else if (m_config.type == HazardType::MagneticStorm) {
                effects.AddTrauma(0.20f);
                // Shift active piece 1 step randomly if valid
                std::uniform_int_distribution<int> dirDist(0, 1);
                int shift = (dirDist(m_rng) == 0) ? -1 : 1;
                piece.TryMove({ shift, 0 }, grid);
                sound.PlayMove();
            }
        }
    } else {
        // Active pulse duration
        m_pulseActiveDuration += dt;
        float maxDuration = (m_config.type == HazardType::GlitchMatrix) ? 3.5f : ((m_config.type == HazardType::GravityFlux) ? 2.5f : 1.2f);

        if (m_pulseActiveDuration >= maxDuration) {
            m_isPulseActive = false;
            m_controlsInverted = false;
            m_gravityMultiplier = (m_config.type == HazardType::SolarFlare) ? 1.35f : 1.0f;
            m_pulseTimer = 0.0f;
        }
    }
}

std::string HazardManager::GetActiveStatusText() const {
    if (m_config.type == HazardType::None) return "";

    if (m_isPulseActive) {
        switch (m_config.type) {
            case HazardType::GravityFlux: return "⚠ GRAVITY PULSE ACTIVE [2.5X FALL SPEED]";
            case HazardType::SingularityCore: return "⚠ SINGULARITY SURGE [GARBAGE INJECTED]";
            case HazardType::GlitchMatrix: return "⚠ GLITCH OVERRIDE ACTIVE [CONTROLS INVERTED!]";
            case HazardType::MagneticStorm: return "⚠ MAGNETIC ANOMALY [LATERAL DISPLACEMENT]";
            case HazardType::SolarFlare: return "⚠ SOLAR FLARE INTENSITY SURGE";
            case HazardType::CryoChamber: return "⚠ CRYO VORTEX ENGAGED";
            default: return "⚠ HAZARD EVENT IN PROGRESS";
        }
    } else {
        float remaining = std::max(0.0f, m_config.pulseInterval - m_pulseTimer);
        char buf[64];
        snprintf(buf, sizeof(buf), "%s (Event in %.1fs)", m_config.title.c_str(), remaining);
        return std::string(buf);
    }
}

} // namespace TetroShift
