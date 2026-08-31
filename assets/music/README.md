# 🎵 TetroShift Studio Soundtrack Assets

This directory hosts high-fidelity stereo `.ogg` (Ogg Vorbis) physical soundtrack files for *TetroShift // MorphoTetris*.

## Track Naming Convention:
1. `01_morpho_awakening.ogg` — Morpho Awakening (Main Menu & Hangar Theme)
2. `02_grid_runner.ogg` — Sector Zero // Grid Runner (Floors 1-3)
3. `03_elasticity_protocol.ogg` — Elasticity Protocol (Floors 4-6)
4. `04_quantum_cascade.ogg` — Quantum Cascade (Floors 7-9)
5. `05_singularity_horizon.ogg` — Singularity Horizon (Floor 10+ Boss Climax)
6. `06_neural_nexus.ogg` — Neural Nexus (Card Draft & Upgrade Theme)
7. `07_memory_purge.ogg` — Memory Purge (Run Summary / Game Over)
8. `08_endless_velocity.ogg` — Endless Velocity (Infinite Marathon Theme)

## Hybrid Audio Pipeline:
- If `.ogg` files are placed here, `MusicManager` automatically loads them via `LoadMusicStream()` with seamless looping and dynamic volume crossfading.
- If `.ogg` files are absent, `MusicManager` gracefully falls back to the in-engine procedural multi-oscillator polyphonic synthesizer at 22.05 kHz.
