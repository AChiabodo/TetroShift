#pragma once
#include <string>
#include <vector>

namespace TetroShift {

enum class TrackId {
    None,
    MenuTheme,          // Morpho Awakening (Main Menu & Hangar)
    EarlyFloorTheme,    // Sector Zero // Grid Runner (Floors 1-3)
    MidFloorTheme,      // Elasticity Protocol (Floors 4-6)
    HighFloorTheme,     // Quantum Cascade (Floors 7-9)
    BossFloorTheme,     // Singularity Horizon (Floors 10+ & Climax)
    DraftTheme,         // Neural Nexus (Card Draft & Upgrade State)
    GameOverTheme,      // Memory Purge (Run Summary / Game Over)
    EndlessTheme        // Endless Velocity (Infinite Marathon)
};

struct TrackMetadata {
    TrackId id = TrackId::None;
    std::string filename;
    std::string title;
    std::string artist;
    std::string genre;
    float baseBpm = 120.0f;
    float defaultVolume = 0.8f;
};

inline std::vector<TrackMetadata> GetDefaultTrackCatalog() {
    return {
        {
            TrackId::MenuTheme,
            "assets/music/01_morpho_awakening.ogg",
            "Morpho Awakening",
            "TetroShift Synth",
            "Neon Synthwave",
            110.0f,
            0.75f
        },
        {
            TrackId::EarlyFloorTheme,
            "assets/music/02_grid_runner.ogg",
            "Sector Zero // Grid Runner",
            "TetroShift Synth",
            "Cyberpunk Electro",
            128.0f,
            0.80f
        },
        {
            TrackId::MidFloorTheme,
            "assets/music/03_elasticity_protocol.ogg",
            "Elasticity Protocol",
            "TetroShift Synth",
            "Funky Glitch Synth",
            134.0f,
            0.80f
        },
        {
            TrackId::HighFloorTheme,
            "assets/music/04_quantum_cascade.ogg",
            "Quantum Cascade",
            "TetroShift Synth",
            "Dark Synthwave",
            142.0f,
            0.85f
        },
        {
            TrackId::BossFloorTheme,
            "assets/music/05_singularity_horizon.ogg",
            "Singularity Horizon",
            "TetroShift Synth",
            "Industrial Darksynth",
            150.0f,
            0.85f
        },
        {
            TrackId::DraftTheme,
            "assets/music/06_neural_nexus.ogg",
            "Neural Nexus",
            "TetroShift Synth",
            "Ambient Downtempo",
            90.0f,
            0.70f
        },
        {
            TrackId::GameOverTheme,
            "assets/music/07_memory_purge.ogg",
            "Memory Purge",
            "TetroShift Synth",
            "80s Synth Requiem",
            85.0f,
            0.75f
        },
        {
            TrackId::EndlessTheme,
            "assets/music/08_endless_velocity.ogg",
            "Endless Velocity",
            "TetroShift Synth",
            "Progressive Trance",
            132.0f,
            0.80f
        }
    };
}

} // namespace TetroShift
