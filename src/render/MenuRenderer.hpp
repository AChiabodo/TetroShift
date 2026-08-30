#pragma once
#include <raylib.h>
#include <string>
#include <vector>
#include "states/MenuTypes.hpp"
#include "piece/TetrominoDefinition.hpp"

namespace TetroShift {

class MenuRenderer {
public:
    MenuRenderer() = default;

    // Background & Atmospherics
    void DrawAnimatedBackground(float timer) const;

    // Navigation & Layout Headers
    void DrawHeaderBanner(const char* title, const char* category, const char* breadcrumb = nullptr) const;
    void DrawPlayerStatusBar(const PlayerProfileData& profile, Rectangle bounds) const;
    void DrawFooterHints(const std::vector<std::string>& hints) const;

    // Interactive Controls
    bool DrawNeonButton(
        Rectangle bounds,
        const char* label,
        const char* badge = nullptr,
        bool isSelected = false,
        bool isHovered = false,
        Color accentColor = { 0, 240, 255, 255 }
    ) const;

    void DrawTabHeader(
        const std::vector<std::string>& tabs,
        int activeTab,
        Rectangle bounds,
        int hoveredTab = -1
    ) const;

    void DrawSlider(
        Rectangle bounds,
        const char* label,
        float value,
        float minVal,
        float maxVal,
        const char* valueText,
        bool isFocused = false
    ) const;

    void DrawToggle(
        Rectangle bounds,
        const char* label,
        bool isActive,
        bool isFocused = false
    ) const;

    // Content Cards & Tables
    void DrawSaveSlotCard(
        Rectangle bounds,
        const SaveSlotData& slot,
        bool isSelected,
        bool isHovered
    ) const;

    void DrawShopItemCard(
        Rectangle bounds,
        const ShopItemData& item,
        int playerCredits,
        bool isSelected,
        bool isHovered
    ) const;

    void DrawScoreRow(
        Rectangle bounds,
        const HighScoreEntry& entry,
        bool isEvenRow,
        bool isHovered
    ) const;

    void DrawMinoSkinPreview(
        TetrominoType type,
        Vector2 center,
        float cellSize,
        float timer,
        Color primaryCol,
        Color secondaryCol
    ) const;

    void DrawModalFrame(Rectangle bounds, const char* title) const;
    void DrawNowPlayingBanner(const char* title, const char* genre, float alpha) const;
};

} // namespace TetroShift
