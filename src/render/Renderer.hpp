#pragma once
#include "grid/IGrid.hpp"
#include "piece/ActivePiece.hpp"
#include "piece/PieceSpawner.hpp"
#include "roguelike/RunManager.hpp"
#include "ParticleSystem.hpp"
#include "ScreenEffects.hpp"
#include <raylib.h>

namespace TetroShift {

class Renderer {
public:
    Renderer() = default;

    void RenderGameHUD(
        const IGrid& grid,
        const ActivePiece& piece,
        const PieceSpawner& spawner,
        const RunManager& runManager,
        const ParticleSystem& particles,
        const ScreenEffects& effects,
        bool showDebugPhysics = false
    ) const;

    void DrawCardUI(const struct Card& card, Rectangle bounds, bool isSelected = false, bool isHovered = false) const;
    void DrawPanelFrame(Rectangle bounds, const char* title = nullptr, Color borderColor = Colors::BgPanelBorder) const;
    void DrawNowPlayingBanner(const char* title, const char* genre, float alpha) const;

private:
    void DrawHoldPanel(const PieceSpawner& spawner, Rectangle bounds) const;
    void DrawNextQueuePanel(const PieceSpawner& spawner, Rectangle bounds) const;
    void DrawStatsPanel(const RunManager& runManager, Rectangle bounds) const;
    void DrawRelicsPanel(const RunManager& runManager, Rectangle bounds) const;
    void DrawControlsPanel(Rectangle bounds) const;
    void DrawMiniTetromino(TetrominoType type, Vector2 center, float cellSize) const;
};

} // namespace TetroShift
