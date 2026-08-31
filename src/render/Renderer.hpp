#pragma once
#include "grid/IGrid.hpp"
#include "piece/ActivePiece.hpp"
#include "piece/PieceSpawner.hpp"
#include "roguelike/RunManager.hpp"
#include "ParticleSystem.hpp"
#include "ScreenEffects.hpp"
#include "states/MenuTypes.hpp"
#include <raylib.h>

#include "FontManager.hpp"

namespace TetroShift {

class Renderer {
public:
    Renderer() = default;

    void SetFontManager(const FontManager* fm) noexcept { m_fontManager = fm; }
    [[nodiscard]] const FontManager* GetFontManager() const noexcept { return m_fontManager; }

    void RenderGameHUD(
        const IGrid& grid,
        const ActivePiece& piece,
        const PieceSpawner& spawner,
        const RunManager& runManager,
        const ParticleSystem& particles,
        const ScreenEffects& effects,
        bool showDebugPhysics = false,
        const class HazardManager* hazardManager = nullptr,
        GameMode gameMode = GameMode::Roguelike,
        int marathonLevel = 1,
        float marathonFallInterval = 1.0f,
        bool sandboxZeroGravity = false,
        float sandboxElasticity = 1.0f,
        int sandboxPiece = 0,
        int sandboxMino = 0,
        const std::string& dailyDate = ""
    ) const;

    void DrawCardUI(const struct Card& card, Rectangle bounds, bool isSelected = false, bool isHovered = false) const;
    void DrawPanelFrame(Rectangle bounds, const char* title = nullptr, Color borderColor = Colors::BgPanelBorder) const;
    void DrawNowPlayingBanner(const char* title, const char* genre, float alpha) const;

private:
    void DrawHoldPanel(const PieceSpawner& spawner, Rectangle bounds) const;
    void DrawNextQueuePanel(const PieceSpawner& spawner, Rectangle bounds) const;
    void DrawStatsPanel(const RunManager& runManager, Rectangle bounds, GameMode gameMode = GameMode::Roguelike, int marathonLevel = 1) const;
    void DrawRelicsPanel(const RunManager& runManager, Rectangle bounds) const;
    void DrawMarathonPanel(int level, float fallInterval, int linesTotal, Rectangle bounds) const;
    void DrawSandboxToolbox(bool zeroGravity, float elasticity, int selectedPiece, int selectedMino, Rectangle bounds) const;
    void DrawControlsPanel(Rectangle bounds, GameMode gameMode = GameMode::Roguelike) const;
    void DrawMiniTetromino(TetrominoType type, Vector2 center, float cellSize) const;

    const FontManager* m_fontManager = nullptr;
};

} // namespace TetroShift
