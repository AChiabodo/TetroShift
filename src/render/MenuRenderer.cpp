#include "MenuRenderer.hpp"
#include "core/Constants.hpp"
#include <cmath>
#include <cstdio>
#include <algorithm>

namespace TetroShift {

void MenuRenderer::DrawAnimatedBackground(float timer) const {
    ClearBackground(Colors::BgDark);

    // 1. Perspective Matrix Grid lines
    const int numCols = 32;
    const float colWidth = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(numCols);
    for (int i = 0; i <= numCols; ++i) {
        float x = static_cast<float>(i) * colWidth;
        float alpha = 0.04f + 0.02f * std::sin(timer * 0.8f + static_cast<float>(i) * 0.2f);
        DrawLineV({ x, 0 }, { x, static_cast<float>(WINDOW_HEIGHT) }, Fade(Colors::PieceI, alpha));
    }

    const int numRows = 20;
    const float rowHeight = static_cast<float>(WINDOW_HEIGHT) / static_cast<float>(numRows);
    for (int j = 0; j <= numRows; ++j) {
        float y = static_cast<float>(j) * rowHeight + std::fmod(timer * 15.0f, rowHeight);
        if (y <= static_cast<float>(WINDOW_HEIGHT)) {
            float alpha = 0.035f + 0.015f * std::cos(timer * 0.7f + static_cast<float>(j) * 0.3f);
            DrawLineV({ 0, y }, { static_cast<float>(WINDOW_WIDTH), y }, Fade(Colors::PieceT, alpha));
        }
    }

    // 2. Ambient Floating Cyber Particles
    for (int i = 0; i < 24; ++i) {
        float seed = static_cast<float>(i);
        float px = std::fmod(seed * 117.0f + timer * (12.0f + std::sin(seed) * 8.0f), static_cast<float>(WINDOW_WIDTH));
        float py = std::fmod(seed * 93.0f + timer * (18.0f + std::cos(seed) * 10.0f), static_cast<float>(WINDOW_HEIGHT));
        float size = 1.5f + std::fmod(seed, 3.0f);
        float pulse = (std::sin(timer * 2.0f + seed) + 1.0f) * 0.5f;
        Color pColor = (i % 3 == 0) ? Colors::PieceI : ((i % 3 == 1) ? Colors::PieceT : Colors::PieceS);
        DrawCircleV({ px, py }, size, Fade(pColor, 0.15f + pulse * 0.25f));
    }

    // 3. Floating Soft Tetrominoes in deep background
    const TetrominoType bgTypes[5] = { TetrominoType::T, TetrominoType::I, TetrominoType::Z, TetrominoType::L, TetrominoType::S };
    for (int i = 0; i < 5; ++i) {
        float tOffset = timer * 0.4f + static_cast<float>(i) * 1.6f;
        float bx = 100.0f + static_cast<float>(i) * 260.0f + std::sin(tOffset) * 45.0f;
        float by = 180.0f + std::cos(tOffset * 0.8f) * 60.0f + (static_cast<float>(i % 2) * 280.0f);
        Color col = GetTetrominoColor(bgTypes[i]);

        // Draw faint glowing silhouette of the tetromino
        const auto coords = TetrominoDefinition::GetMinoCoords(bgTypes[i], 0);
        for (const auto& c : coords) {
            float minoX = bx + static_cast<float>(c.x) * 28.0f;
            float minoY = by + static_cast<float>(c.y) * 28.0f;
            DrawRectangleRounded({ minoX, minoY, 26.0f, 26.0f }, 0.2f, 4, Fade(col, 0.05f));
            DrawRectangleLinesEx({ minoX, minoY, 26.0f, 26.0f }, 1.2f, Fade(col, 0.12f));
        }
    }

    // Subtle dark vignette at borders
    DrawRectangleGradientV(0, 0, WINDOW_WIDTH, 90, Fade(BLACK, 0.85f), Fade(BLACK, 0.0f));
    DrawRectangleGradientV(0, WINDOW_HEIGHT - 90, WINDOW_WIDTH, 90, Fade(BLACK, 0.0f), Fade(BLACK, 0.85f));
    DrawRectangleGradientH(0, 0, 90, WINDOW_HEIGHT, Fade(BLACK, 0.85f), Fade(BLACK, 0.0f));
    DrawRectangleGradientH(WINDOW_WIDTH - 90, 0, 90, WINDOW_HEIGHT, Fade(BLACK, 0.0f), Fade(BLACK, 0.85f));
}

void MenuRenderer::DrawHeaderBanner(const char* title, const char* category, const char* breadcrumb) const {
    // Breadcrumb path
    std::string path = "TETROSHIFT // ";
    if (breadcrumb && breadcrumb[0] != '\0') {
        path += breadcrumb;
        path += " // ";
    }
    if (category) {
        path += category;
    }
    DrawText(path.c_str(), 48, 28, 12, Colors::TextDim);

    // Main Title
    if (title) {
        DrawText(title, 48, 46, 32, Colors::TextWhite);
        // Underline glowing accent
        int titleW = MeasureText(title, 32);
        DrawLineEx({ 48.0f, 84.0f }, { 48.0f + static_cast<float>(titleW) + 40.0f, 84.0f }, 2.5f, Colors::PieceI);
        DrawLineEx({ 48.0f + static_cast<float>(titleW) + 45.0f, 84.0f }, { 48.0f + static_cast<float>(titleW) + 65.0f, 84.0f }, 2.5f, Colors::PieceT);
    }
}

void MenuRenderer::DrawPlayerStatusBar(const PlayerProfileData& profile, Rectangle bounds) const {
    // Capsule frame
    DrawRectangleRounded(bounds, 0.2f, 6, Colors::BgPanel);
    DrawRectangleLinesEx(bounds, 1.5f, Colors::BgPanelBorder);

    // Pilot Icon / Avatar box
    Rectangle avatarBox = { bounds.x + 8.0f, bounds.y + (bounds.height - 44.0f) * 0.5f, 44.0f, 44.0f };
    DrawRectangleRounded(avatarBox, 0.2f, 4, Fade(Colors::PieceI, 0.15f));
    DrawRectangleLinesEx(avatarBox, 1.5f, Colors::PieceI);
    DrawText("P-1", static_cast<int>(avatarBox.x + 9.0f), static_cast<int>(avatarBox.y + 14.0f), 15, Colors::TextAccent);

    // Energy Credits Capsule on the right
    float credWidth = 116.0f;
    float credHeight = 40.0f;
    Rectangle credBox = { bounds.x + bounds.width - credWidth - 10.0f, bounds.y + (bounds.height - credHeight) * 0.5f, credWidth, credHeight };
    DrawRectangleRounded(credBox, 0.2f, 4, Fade(Colors::PieceGold, 0.12f));
    DrawRectangleLinesEx(credBox, 1.2f, Colors::PieceGold);
    DrawText("CREDITS", static_cast<int>(credBox.x + 8.0f), static_cast<int>(credBox.y + 5.0f), 9, Colors::TextDim);
    std::string credStr = "$" + std::to_string(profile.energyCredits);
    DrawText(credStr.c_str(), static_cast<int>(credBox.x + 8.0f), static_cast<int>(credBox.y + 17.0f), 15, Colors::TextGold);

    // Pilot Name & Rank in the middle
    int tx = static_cast<int>(bounds.x + 60.0f);
    DrawText(profile.pilotName.c_str(), tx, static_cast<int>(bounds.y + 11.0f), 13, Colors::TextWhite);
    std::string rankStr = "LVL " + std::to_string(profile.level) + " * " + profile.rankTitle;
    DrawText(rankStr.c_str(), tx, static_cast<int>(bounds.y + 27.0f), 10, Colors::TextDim);

    // XP Progress Bar (dynamically stretches up to credits box)
    float availableXpWidth = (credBox.x - 14.0f) - static_cast<float>(tx);
    if (availableXpWidth > 40.0f) {
        float xpProgress = (profile.maxExp > 0) ? (static_cast<float>(profile.currentExp) / static_cast<float>(profile.maxExp)) : 0.0f;
        xpProgress = std::min(1.0f, std::max(0.0f, xpProgress));
        Rectangle xpBarBg = { static_cast<float>(tx), bounds.y + 44.0f, availableXpWidth, 6.0f };
        DrawRectangleRounded(xpBarBg, 0.5f, 4, Colors::GridBg);
        if (xpProgress > 0.0f) {
            Rectangle xpBarFill = { xpBarBg.x, xpBarBg.y, xpBarBg.width * xpProgress, xpBarBg.height };
            DrawRectangleRounded(xpBarFill, 0.5f, 4, Colors::PieceI);
        }
    }
}

void MenuRenderer::DrawFooterHints(const std::vector<std::string>& hints) const {
    Rectangle barRect = { 0, WINDOW_HEIGHT - 44.0f, static_cast<float>(WINDOW_WIDTH), 44.0f };
    DrawRectangleRec(barRect, Fade(Colors::BgDark, 0.92f));
    DrawLine(0, static_cast<int>(barRect.y), WINDOW_WIDTH, static_cast<int>(barRect.y), Colors::BgPanelBorder);

    float currX = 48.0f;
    for (const auto& hint : hints) {
        int textW = MeasureText(hint.c_str(), 12);
        Rectangle pill = { currX, barRect.y + 9.0f, static_cast<float>(textW + 18.0f), 26.0f };
        DrawRectangleRounded(pill, 0.3f, 4, Colors::BgPanel);
        DrawRectangleLinesEx(pill, 1.0f, Colors::BgPanelBorder);
        DrawText(hint.c_str(), static_cast<int>(pill.x + 9.0f), static_cast<int>(pill.y + 7.0f), 12, Colors::TextDim);
        currX += pill.width + 16.0f;
    }
}

bool MenuRenderer::DrawNeonButton(
    Rectangle bounds,
    const char* label,
    const char* badge,
    bool isSelected,
    bool isHovered,
    Color accentColor
) const {
    Color bg = Colors::BgPanel;
    Color border = Colors::BgPanelBorder;
    Color textColor = Colors::TextWhite;

    if (isSelected) {
        bg = Fade(accentColor, 0.22f);
        border = accentColor;
        textColor = Colors::TextWhite;
    } else if (isHovered) {
        bg = Fade(Colors::BgPanelBorder, 0.6f);
        border = Fade(accentColor, 0.8f);
        textColor = accentColor;
    }

    // Outer glow for selected
    if (isSelected) {
        Rectangle glowRect = { bounds.x - 2.0f, bounds.y - 2.0f, bounds.width + 4.0f, bounds.height + 4.0f };
        DrawRectangleRounded(glowRect, 0.18f, 6, Fade(accentColor, 0.15f));
    }

    DrawRectangleRounded(bounds, 0.16f, 6, bg);
    DrawRectangleLinesEx(bounds, isSelected ? 2.5f : (isHovered ? 2.0f : 1.2f), border);

    // Left active indicator strip
    if (isSelected) {
        Rectangle strip = { bounds.x + 3.0f, bounds.y + 6.0f, 4.0f, bounds.height - 12.0f };
        DrawRectangleRounded(strip, 0.5f, 2, accentColor);
    }

    // Label
    int textY = static_cast<int>(bounds.y + (bounds.height - 18.0f) * 0.5f);
    DrawText(label, static_cast<int>(bounds.x + 22.0f), textY, 17, textColor);

    // Badge / Extra info on the right
    if (badge) {
        int badgeW = MeasureText(badge, 11);
        Rectangle badgeRect = { bounds.x + bounds.width - static_cast<float>(badgeW) - 24.0f, bounds.y + (bounds.height - 20.0f) * 0.5f, static_cast<float>(badgeW + 14.0f), 20.0f };
        DrawRectangleRounded(badgeRect, 0.3f, 4, isSelected ? Fade(accentColor, 0.3f) : Colors::GridBg);
        DrawRectangleLinesEx(badgeRect, 1.0f, isSelected ? accentColor : Colors::BgPanelBorder);
        DrawText(badge, static_cast<int>(badgeRect.x + 7.0f), static_cast<int>(badgeRect.y + 4.0f), 11, isSelected ? accentColor : Colors::TextDim);
    }

    return isSelected || isHovered;
}

void MenuRenderer::DrawTabHeader(
    const std::vector<std::string>& tabs,
    int activeTab,
    Rectangle bounds,
    int hoveredTab
) const {
    if (tabs.empty()) return;
    float tabWidth = bounds.width / static_cast<float>(tabs.size());

    for (size_t i = 0; i < tabs.size(); ++i) {
        Rectangle tabRect = { bounds.x + static_cast<float>(i) * tabWidth, bounds.y, tabWidth - 6.0f, bounds.height };
        bool isActive = (static_cast<int>(i) == activeTab);
        bool isHovered = (static_cast<int>(i) == hoveredTab);

        Color bg = isActive ? Fade(Colors::PieceI, 0.18f) : (isHovered ? Fade(Colors::BgPanelBorder, 0.5f) : Colors::BgPanel);
        Color border = isActive ? Colors::PieceI : (isHovered ? Colors::TextDim : Colors::BgPanelBorder);

        DrawRectangleRounded(tabRect, 0.15f, 4, bg);
        DrawRectangleLinesEx(tabRect, isActive ? 2.0f : 1.0f, border);

        int textW = MeasureText(tabs[i].c_str(), 13);
        DrawText(
            tabs[i].c_str(),
            static_cast<int>(tabRect.x + (tabRect.width - textW) * 0.5f),
            static_cast<int>(tabRect.y + (tabRect.height - 13.0f) * 0.5f),
            13,
            isActive ? Colors::TextWhite : Colors::TextDim
        );
    }
}

void MenuRenderer::DrawSlider(
    Rectangle bounds,
    const char* label,
    float value,
    float minVal,
    float maxVal,
    const char* valueText,
    bool isFocused
) const {
    DrawRectangleRounded(bounds, 0.12f, 4, isFocused ? Fade(Colors::PieceI, 0.1f) : Colors::BgPanel);
    DrawRectangleLinesEx(bounds, isFocused ? 2.0f : 1.0f, isFocused ? Colors::PieceI : Colors::BgPanelBorder);

    // Label
    DrawText(label, static_cast<int>(bounds.x + 16.0f), static_cast<int>(bounds.y + 14.0f), 14, isFocused ? Colors::TextWhite : Colors::TextDim);

    // Slider track
    float trackX = bounds.x + 220.0f;
    float trackY = bounds.y + bounds.height * 0.5f - 4.0f;
    float trackWidth = bounds.width - 340.0f;
    float trackHeight = 8.0f;

    Rectangle trackRect = { trackX, trackY, trackWidth, trackHeight };
    DrawRectangleRounded(trackRect, 0.5f, 4, Colors::GridBg);

    float norm = (maxVal > minVal) ? ((value - minVal) / (maxVal - minVal)) : 0.0f;
    norm = std::min(1.0f, std::max(0.0f, norm));

    Rectangle fillRect = { trackX, trackY, trackWidth * norm, trackHeight };
    DrawRectangleRounded(fillRect, 0.5f, 4, isFocused ? Colors::TextGreen : Colors::PieceI);

    // Thumb handle
    float thumbX = trackX + trackWidth * norm;
    DrawCircle(static_cast<int>(thumbX), static_cast<int>(trackY + 4.0f), 8.0f, WHITE);
    DrawCircle(static_cast<int>(thumbX), static_cast<int>(trackY + 4.0f), 6.0f, isFocused ? Colors::TextGreen : Colors::PieceI);

    // Value Text readout
    if (valueText) {
        int vW = MeasureText(valueText, 14);
        DrawText(valueText, static_cast<int>(bounds.x + bounds.width - vW - 18.0f), static_cast<int>(bounds.y + 14.0f), 14, Colors::TextWhite);
    }
}

void MenuRenderer::DrawToggle(
    Rectangle bounds,
    const char* label,
    bool isActive,
    bool isFocused
) const {
    DrawRectangleRounded(bounds, 0.12f, 4, isFocused ? Fade(Colors::PieceI, 0.1f) : Colors::BgPanel);
    DrawRectangleLinesEx(bounds, isFocused ? 2.0f : 1.0f, isFocused ? Colors::PieceI : Colors::BgPanelBorder);

    DrawText(label, static_cast<int>(bounds.x + 16.0f), static_cast<int>(bounds.y + (bounds.height - 14.0f) * 0.5f), 14, isFocused ? Colors::TextWhite : Colors::TextDim);

    // Toggle switch pill
    Rectangle switchRect = { bounds.x + bounds.width - 80.0f, bounds.y + (bounds.height - 24.0f) * 0.5f, 62.0f, 24.0f };
    DrawRectangleRounded(switchRect, 0.5f, 4, isActive ? Fade(Colors::TextGreen, 0.3f) : Colors::GridBg);
    DrawRectangleLinesEx(switchRect, 1.2f, isActive ? Colors::TextGreen : Colors::BgPanelBorder);

    float thumbX = isActive ? (switchRect.x + switchRect.width - 12.0f) : (switchRect.x + 12.0f);
    DrawCircle(static_cast<int>(thumbX), static_cast<int>(switchRect.y + 12.0f), 8.0f, isActive ? Colors::TextGreen : Colors::TextDim);

    const char* stateStr = isActive ? "ON" : "OFF";
    DrawText(stateStr, static_cast<int>(switchRect.x + (isActive ? 12.0f : 28.0f)), static_cast<int>(switchRect.y + 7.0f), 10, isActive ? Colors::TextGreen : Colors::TextDim);
}

void MenuRenderer::DrawSaveSlotCard(
    Rectangle bounds,
    const SaveSlotData& slot,
    bool isSelected,
    bool isHovered
) const {
    Color border = isSelected ? Colors::PieceI : (isHovered ? WHITE : Colors::BgPanelBorder);
    Color bg = isSelected ? Fade(Colors::PieceI, 0.12f) : (isHovered ? Fade(Colors::BgPanelBorder, 0.4f) : Colors::BgPanel);

    DrawRectangleRounded(bounds, 0.08f, 6, bg);
    DrawRectangleLinesEx(bounds, isSelected ? 2.5f : 1.5f, border);

    // Slot Header Tag
    Rectangle tagRect = { bounds.x + 14.0f, bounds.y + 14.0f, 90.0f, 24.0f };
    DrawRectangleRounded(tagRect, 0.2f, 4, Fade(slot.accentColor, 0.2f));
    DrawRectangleLinesEx(tagRect, 1.0f, slot.accentColor);
    std::string tagStr = "SLOT 0" + std::to_string(slot.slotId);
    DrawText(tagStr.c_str(), static_cast<int>(tagRect.x + 12.0f), static_cast<int>(tagRect.y + 5.0f), 12, slot.accentColor);

    // Date
    DrawText(slot.timestamp.c_str(), static_cast<int>(bounds.x + bounds.width - 120.0f), static_cast<int>(bounds.y + 18.0f), 11, Colors::TextDim);

    if (slot.state == SaveSlotState::Empty) {
        DrawText("EMPTY SAVE MATRIX", static_cast<int>(bounds.x + 20.0f), static_cast<int>(bounds.y + 60.0f), 18, Colors::TextDim);
        DrawText("Ready for new mission departure. Press [ENTER] to start.", static_cast<int>(bounds.x + 20.0f), static_cast<int>(bounds.y + 90.0f), 12, Colors::TextDim);
    } else {
        // Mode Title
        DrawText(slot.runMode.c_str(), static_cast<int>(bounds.x + 14.0f), static_cast<int>(bounds.y + 48.0f), 18, Colors::TextWhite);

        // Stats summary
        int sy = static_cast<int>(bounds.y + 76.0f);
        std::string floorStr = "FLOOR: " + std::to_string(slot.currentFloor);
        DrawText(floorStr.c_str(), static_cast<int>(bounds.x + 14.0f), sy, 13, Colors::TextAccent);

        std::string scoreStr = "SCORE: " + std::to_string(slot.currentScore);
        DrawText(scoreStr.c_str(), static_cast<int>(bounds.x + 120.0f), sy, 13, Colors::TextGold);

        std::string linesStr = "LINES: " + std::to_string(slot.currentLines);
        DrawText(linesStr.c_str(), static_cast<int>(bounds.x + 250.0f), sy, 13, Colors::TextWhite);

        // Relics mini badges
        if (!slot.relicCards.empty()) {
            DrawText("ACTIVE MODIFIERS:", static_cast<int>(bounds.x + 14.0f), sy + 24, 10, Colors::TextDim);
            float pillX = bounds.x + 130.0f;
            for (size_t i = 0; i < slot.relicCards.size() && i < 3; ++i) {
                int pw = MeasureText(slot.relicCards[i].c_str(), 10) + 12;
                Rectangle relicPill = { pillX, static_cast<float>(sy + 20), static_cast<float>(pw), 18.0f };
                DrawRectangleRounded(relicPill, 0.3f, 4, Colors::GridBg);
                DrawRectangleLinesEx(relicPill, 1.0f, Colors::PieceT);
                DrawText(slot.relicCards[i].c_str(), static_cast<int>(pillX + 6.0f), sy + 24, 10, Colors::TextWhite);
                pillX += pw + 8.0f;
            }
        }
    }
}

void MenuRenderer::DrawShopItemCard(
    Rectangle bounds,
    const ShopItemData& item,
    int playerCredits,
    bool isSelected,
    bool isHovered
) const {
    Color border = isSelected ? item.rarityColor : (isHovered ? WHITE : Colors::BgPanelBorder);
    Color bg = isSelected ? Fade(item.rarityColor, 0.15f) : (isHovered ? Fade(Colors::BgPanelBorder, 0.5f) : Colors::BgPanel);

    DrawRectangleRounded(bounds, 0.08f, 6, bg);
    DrawRectangleLinesEx(bounds, isSelected ? 2.5f : 1.2f, border);

    // Category Tag
    Rectangle tagRect = { bounds.x + 10.0f, bounds.y + 10.0f, 85.0f, 20.0f };
    DrawRectangleRounded(tagRect, 0.2f, 4, Fade(item.rarityColor, 0.25f));
    DrawText(item.categoryName.c_str(), static_cast<int>(tagRect.x + 6.0f), static_cast<int>(tagRect.y + 4.0f), 10, item.rarityColor);

    // Cost / Status tag on right
    if (item.isEquipped) {
        DrawText("EQUIPPED", static_cast<int>(bounds.x + bounds.width - 80.0f), static_cast<int>(bounds.y + 12.0f), 11, Colors::TextGreen);
    } else if (item.isUnlocked) {
        DrawText("UNLOCKED", static_cast<int>(bounds.x + bounds.width - 80.0f), static_cast<int>(bounds.y + 12.0f), 11, Colors::TextAccent);
    } else {
        std::string costStr = "$" + std::to_string(item.cost);
        Color costColor = (playerCredits >= item.cost) ? Colors::TextGold : Colors::TextDanger;
        DrawText(costStr.c_str(), static_cast<int>(bounds.x + bounds.width - 65.0f), static_cast<int>(bounds.y + 12.0f), 13, costColor);
    }

    // Name
    DrawText(item.name.c_str(), static_cast<int>(bounds.x + 12.0f), static_cast<int>(bounds.y + 36.0f), 15, Colors::TextWhite);

    // Divider
    DrawLine(static_cast<int>(bounds.x + 10.0f), static_cast<int>(bounds.y + 58.0f),
             static_cast<int>(bounds.x + bounds.width - 10.0f), static_cast<int>(bounds.y + 58.0f), Colors::BgPanelBorder);

    // Description
    DrawText(item.description.c_str(), static_cast<int>(bounds.x + 12.0f), static_cast<int>(bounds.y + 66.0f), 11, Colors::TextDim);
}

void MenuRenderer::DrawScoreRow(
    Rectangle bounds,
    const HighScoreEntry& entry,
    bool isEvenRow,
    bool isHovered
) const {
    Color bg = isHovered ? Fade(Colors::PieceI, 0.15f) : (isEvenRow ? Colors::BgPanel : Fade(Colors::BgPanel, 0.6f));
    DrawRectangleRounded(bounds, 0.05f, 4, bg);
    if (isHovered) {
        DrawRectangleLinesEx(bounds, 1.5f, Colors::PieceI);
    }

    int y = static_cast<int>(bounds.y + (bounds.height - 14.0f) * 0.5f);

    // Rank Medal / Number
    std::string rankStr = "#" + std::to_string(entry.rank);
    Color rankColor = (entry.rank == 1) ? GOLD : ((entry.rank == 2) ? LIGHTGRAY : ((entry.rank == 3) ? Colors::PieceL : Colors::TextDim));
    DrawText(rankStr.c_str(), static_cast<int>(bounds.x + 16.0f), y, 14, rankColor);

    // Pilot
    DrawText(entry.pilotName.c_str(), static_cast<int>(bounds.x + 80.0f), y, 14, Colors::TextWhite);

    // Score
    std::string scoreStr = std::to_string(entry.score);
    DrawText(scoreStr.c_str(), static_cast<int>(bounds.x + 280.0f), y, 15, Colors::TextGold);

    // Floor
    std::string floorStr = "FL " + std::to_string(entry.floorReached);
    DrawText(floorStr.c_str(), static_cast<int>(bounds.x + 440.0f), y, 13, Colors::TextAccent);

    // Lines
    std::string linesStr = std::to_string(entry.linesCleared) + " L";
    DrawText(linesStr.c_str(), static_cast<int>(bounds.x + 550.0f), y, 13, Colors::TextDim);

    // Badge
    Rectangle badgeRect = { bounds.x + bounds.width - 130.0f, bounds.y + (bounds.height - 20.0f) * 0.5f, 110.0f, 20.0f };
    DrawRectangleRounded(badgeRect, 0.3f, 4, Fade(entry.badgeColor, 0.2f));
    DrawRectangleLinesEx(badgeRect, 1.0f, entry.badgeColor);
    DrawText(entry.badge.c_str(), static_cast<int>(badgeRect.x + 8.0f), static_cast<int>(badgeRect.y + 4.0f), 10, entry.badgeColor);
}

void MenuRenderer::DrawMinoSkinPreview(
    TetrominoType type,
    Vector2 center,
    float cellSize,
    float timer,
    Color primaryCol,
    Color secondaryCol
) const {
    const auto coords = TetrominoDefinition::GetMinoCoords(type, 0);

    // Compute bounding box
    float minX = 99.0f, maxX = -99.0f, minY = 99.0f, maxY = -99.0f;
    for (const auto& c : coords) {
        if (static_cast<float>(c.x) < minX) minX = static_cast<float>(c.x);
        if (static_cast<float>(c.x) > maxX) maxX = static_cast<float>(c.x);
        if (static_cast<float>(c.y) < minY) minY = static_cast<float>(c.y);
        if (static_cast<float>(c.y) > maxY) maxY = static_cast<float>(c.y);
    }

    const float width = (maxX - minX + 1.0f) * cellSize;
    const float height = (maxY - minY + 1.0f) * cellSize;
    const Vector2 startPos = { center.x - width * 0.5f - minX * cellSize, center.y - height * 0.5f - minY * cellSize };

    for (size_t i = 0; i < coords.size(); ++i) {
        const auto& c = coords[i];
        // Subtle harmonic wobble on each block
        float wobbleOffset = std::sin(timer * 4.0f + static_cast<float>(i) * 1.2f) * 3.0f;

        Rectangle r = {
            startPos.x + static_cast<float>(c.x) * cellSize + 2.0f,
            startPos.y + static_cast<float>(c.y) * cellSize + 2.0f + wobbleOffset,
            cellSize - 4.0f,
            cellSize - 4.0f
        };

        // Outer glow
        Rectangle glow = { r.x - 2.0f, r.y - 2.0f, r.width + 4.0f, r.height + 4.0f };
        DrawRectangleRounded(glow, 0.25f, 4, Fade(secondaryCol, 0.2f));

        DrawRectangleRounded(r, 0.25f, 4, primaryCol);
        // Bevel highlight
        DrawRectangleRec({ r.x + 2.0f, r.y + 2.0f, r.width - 4.0f, 3.0f }, Fade(WHITE, 0.6f));
        DrawRectangleLinesEx(r, 1.5f, secondaryCol);
    }
}

void MenuRenderer::DrawModalFrame(Rectangle bounds, const char* title) const {
    // Backdrop blur / dim
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.7f));

    // Modal Card
    DrawRectangleRounded(bounds, 0.1f, 6, Colors::BgDark);
    DrawRectangleLinesEx(bounds, 2.0f, Colors::PieceBomb);

    // Header strip
    Rectangle headerRect = { bounds.x + 2.0f, bounds.y + 2.0f, bounds.width - 4.0f, 40.0f };
    DrawRectangleRounded(headerRect, 0.1f, 4, Fade(Colors::PieceBomb, 0.2f));
    DrawText(title, static_cast<int>(headerRect.x + 18.0f), static_cast<int>(headerRect.y + 12.0f), 16, Colors::TextWhite);
}

} // namespace TetroShift
