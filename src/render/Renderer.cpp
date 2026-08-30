#include "Renderer.hpp"
#include "core/Constants.hpp"
#include "roguelike/Card.hpp"
#include <string>
#include <cmath>

namespace TetroShift {

void Renderer::DrawPanelFrame(Rectangle bounds, const char* title, Color borderColor) const {
    // Fill panel background
    DrawRectangleRounded(bounds, 0.08f, 6, Colors::BgPanel);
    // Border line
    DrawRectangleLinesEx(bounds, 2.0f, borderColor);

    if (title) {
        // Title banner
        DrawRectangle(static_cast<int>(bounds.x + 8.0f), static_cast<int>(bounds.y - 10.0f),
                      MeasureText(title, 14) + 12, 20, Colors::BgDark);
        DrawText(title, static_cast<int>(bounds.x + 14.0f), static_cast<int>(bounds.y - 7.0f), 14, Colors::TextAccent);
    }
}

void Renderer::DrawMiniTetromino(TetrominoType type, Vector2 center, float cellSize) const {
    const auto coords = TetrominoDefinition::GetMinoCoords(type, 0);
    const Color color = GetTetrominoColor(type);

    // Compute bounding center
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

    for (const auto& c : coords) {
        Rectangle r = {
            startPos.x + static_cast<float>(c.x) * cellSize + 1.0f,
            startPos.y + static_cast<float>(c.y) * cellSize + 1.0f,
            cellSize - 2.0f,
            cellSize - 2.0f
        };
        DrawRectangleRounded(r, 0.25f, 4, color);
        DrawRectangleRec({ r.x + 1.0f, r.y + 1.0f, r.width - 2.0f, 2.0f }, Fade(WHITE, 0.4f));
    }
}

void Renderer::DrawHoldPanel(const PieceSpawner& spawner, Rectangle bounds) const {
    DrawPanelFrame(bounds, "HOLD [C / SHIFT]");

    const auto& hold = spawner.GetHoldPiece();
    if (hold.has_value()) {
        Color tint = spawner.CanHold() ? WHITE : Fade(GRAY, 0.6f);
        Vector2 center = { bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.55f };
        DrawMiniTetromino(hold.value(), center, 22.0f);
        if (!spawner.CanHold()) {
            DrawText("LOCKED", static_cast<int>(bounds.x + bounds.width * 0.5f - 24.0f), static_cast<int>(bounds.y + bounds.height - 18.0f), 10, RED);
        }
    } else {
        DrawText("EMPTY", static_cast<int>(bounds.x + bounds.width * 0.5f - 20.0f), static_cast<int>(bounds.y + bounds.height * 0.5f - 6.0f), 12, Colors::TextDim);
    }
}

void Renderer::DrawNextQueuePanel(const PieceSpawner& spawner, Rectangle bounds) const {
    DrawPanelFrame(bounds, "NEXT QUEUE");

    const auto queue = spawner.PeekNextQueue(4);
    float slotHeight = (bounds.height - 30.0f) / 4.0f;

    for (size_t i = 0; i < queue.size(); ++i) {
        Vector2 center = { bounds.x + bounds.width * 0.5f, bounds.y + 25.0f + static_cast<float>(i) * slotHeight + slotHeight * 0.5f };
        float miniCell = (i == 0) ? 22.0f : 18.0f; // First next piece is slightly larger
        DrawMiniTetromino(queue[i], center, miniCell);
    }
}

void Renderer::DrawStatsPanel(const RunManager& runManager, Rectangle bounds) const {
    DrawPanelFrame(bounds, "RUN STATS");

    int textX = static_cast<int>(bounds.x + 16.0f);
    int textY = static_cast<int>(bounds.y + 22.0f);
    int spacing = 26;

    // Score
    DrawText("SCORE", textX, textY, 12, Colors::TextDim);
    std::string scoreStr = std::to_string(runManager.GetScore());
    DrawText(scoreStr.c_str(), textX, textY + 13, 20, Colors::TextWhite);
    textY += spacing + 12;

    // Floor / Level
    DrawText("FLOOR / LEVEL", textX, textY, 12, Colors::TextDim);
    std::string floorStr = "FLOOR " + std::to_string(runManager.GetFloor());
    DrawText(floorStr.c_str(), textX, textY + 13, 18, Colors::TextAccent);
    textY += spacing + 10;

    // Floor Objective Progress
    DrawText("FLOOR PROGRESS", textX, textY, 12, Colors::TextDim);
    std::string progStr = std::to_string(runManager.GetLinesThisFloor()) + " / " + std::to_string(runManager.GetFloorLineTarget()) + " LINES";
    DrawText(progStr.c_str(), textX, textY + 13, 12, Colors::TextWhite);

    // Progress Bar
    float progress = static_cast<float>(runManager.GetLinesThisFloor()) / static_cast<float>(std::max(1, runManager.GetFloorLineTarget()));
    progress = std::min(1.0f, progress);
    Rectangle barBg = { static_cast<float>(textX), static_cast<float>(textY + 28), bounds.width - 32.0f, 10.0f };
    DrawRectangleRounded(barBg, 0.4f, 4, Colors::GridBg);
    if (progress > 0.0f) {
        Rectangle barFill = { barBg.x, barBg.y, barBg.width * progress, barBg.height };
        DrawRectangleRounded(barFill, 0.4f, 4, (progress >= 1.0f) ? GOLD : Colors::TextGreen);
    }
    textY += spacing + 18;

    // Coins & Score Multiplier
    DrawText("COINS", textX, textY, 12, Colors::TextDim);
    std::string coinsStr = "$" + std::to_string(runManager.GetInventory().GetCoins());
    DrawText(coinsStr.c_str(), textX, textY + 13, 18, Colors::TextGold);

    DrawText("MULT", textX + 110, textY, 12, Colors::TextDim);
    char multBuffer[16];
    snprintf(multBuffer, sizeof(multBuffer), "x%.2f", runManager.GetScoreMultiplier());
    DrawText(multBuffer, textX + 110, textY + 13, 18, Colors::TextGreen);
    textY += spacing + 12;

    // Combo
    if (runManager.GetCombo() > 0) {
        std::string comboStr = "COMBO x" + std::to_string(runManager.GetCombo()) + "!";
        DrawText(comboStr.c_str(), textX, textY, 16, Colors::PieceBomb);
    }
}

void Renderer::DrawRelicsPanel(const RunManager& runManager, Rectangle bounds) const {
    DrawPanelFrame(bounds, "ACTIVE ABILITIES & RELICS");

    const auto& actives = runManager.GetInventory().GetActives();
    const auto& passives = runManager.GetInventory().GetPassives();

    float currY = bounds.y + 20.0f;

    // Active Abilities section
    if (!actives.empty()) {
        DrawText("ACTIVE ABILITIES [KEY 1 / 2]:", static_cast<int>(bounds.x + 12.0f), static_cast<int>(currY), 11, Colors::TextDim);
        currY += 16.0f;

        for (size_t i = 0; i < actives.size(); ++i) {
            const auto& act = actives[i];
            std::string keyLabel = "[" + std::to_string(i + 1) + "] " + act.title;
            DrawText(keyLabel.c_str(), static_cast<int>(bounds.x + 12.0f), static_cast<int>(currY), 12, Colors::TextAccent);

            if (act.currentCooldown > 0) {
                std::string cdText = "(CD: " + std::to_string(act.currentCooldown) + " lines)";
                DrawText(cdText.c_str(), static_cast<int>(bounds.x + 160.0f), static_cast<int>(currY), 11, Colors::TextDanger);
            } else {
                DrawText("READY!", static_cast<int>(bounds.x + 160.0f), static_cast<int>(currY), 11, Colors::TextGreen);
            }
            currY += 18.0f;
        }
        currY += 8.0f;
    }

    // Passive Relics section
    std::string passivesTitle = "PASSIVE MODIFIERS (" + std::to_string(passives.size()) + "):";
    DrawText(passivesTitle.c_str(), static_cast<int>(bounds.x + 12.0f), static_cast<int>(currY), 11, Colors::TextDim);
    currY += 16.0f;

    if (passives.empty()) {
        DrawText("None acquired yet. Draft at floor goal!", static_cast<int>(bounds.x + 12.0f), static_cast<int>(currY), 11, Colors::TextDim);
    } else {
        for (const auto& p : passives) {
            if (currY > bounds.y + bounds.height - 24.0f) break;

            Color rarityCol = p.GetRarityColor();
            DrawCircle(static_cast<int>(bounds.x + 18.0f), static_cast<int>(currY + 6.0f), 4.0f, rarityCol);
            DrawText(p.title.c_str(), static_cast<int>(bounds.x + 28.0f), static_cast<int>(currY), 12, Colors::TextWhite);
            currY += 18.0f;
        }
    }
}

void Renderer::DrawControlsPanel(Rectangle bounds) const {
    DrawPanelFrame(bounds, "CONTROLS");

    int tx = static_cast<int>(bounds.x + 12.0f);
    int ty = static_cast<int>(bounds.y + 18.0f);

    DrawText("LEFT / RIGHT / DOWN : Move & Soft Drop", tx, ty, 10, Colors::TextDim); ty += 15;
    DrawText("UP / X : Rotate CW  |  Z : Rotate CCW", tx, ty, 10, Colors::TextDim); ty += 15;
    DrawText("SPACE : Hard Drop   |  C : Hold", tx, ty, 10, Colors::TextDim); ty += 15;
    DrawText("1 / 2 : Use Active  |  P : Pause", tx, ty, 10, Colors::TextDim); ty += 15;
    DrawText("F1: Physics  F2: Draft  F3: Clear  F5: Reset", tx, ty, 10, Colors::TextAccent);
}

void Renderer::DrawCardUI(const Card& card, Rectangle bounds, bool isSelected, bool isHovered) const {
    Color rarityCol = card.GetRarityColor();
    Color bgCol = Colors::BgPanel;

    if (isSelected) {
        bgCol = Fade(rarityCol, 0.25f);
    } else if (isHovered) {
        bgCol = Fade(Colors::BgPanelBorder, 0.8f);
    }

    // Card background
    DrawRectangleRounded(bounds, 0.08f, 6, bgCol);
    DrawRectangleLinesEx(bounds, isSelected ? 3.0f : (isHovered ? 2.5f : 1.5f), isSelected ? GOLD : (isHovered ? WHITE : rarityCol));

    // Rarity Header Tag
    Rectangle headerRect = { bounds.x + 4.0f, bounds.y + 4.0f, bounds.width - 8.0f, 24.0f };
    DrawRectangleRounded(headerRect, 0.2f, 4, Fade(rarityCol, 0.35f));
    DrawText(card.GetRarityName(), static_cast<int>(headerRect.x + 8.0f), static_cast<int>(headerRect.y + 5.0f), 12, rarityCol);

    if (card.cost > 0) {
        std::string costStr = "$" + std::to_string(card.cost);
        DrawText(costStr.c_str(), static_cast<int>(headerRect.x + headerRect.width - 36.0f), static_cast<int>(headerRect.y + 5.0f), 12, GOLD);
    }

    // Title
    DrawText(card.title.c_str(), static_cast<int>(bounds.x + 12.0f), static_cast<int>(bounds.y + 36.0f), 16, Colors::TextWhite);

    // Divider
    DrawLine(static_cast<int>(bounds.x + 10.0f), static_cast<int>(bounds.y + 58.0f),
             static_cast<int>(bounds.x + bounds.width - 10.0f), static_cast<int>(bounds.y + 58.0f), Fade(rarityCol, 0.5f));

    // Description text (multi-line wrapping)
    int descY = static_cast<int>(bounds.y + 68.0f);
    // Simple word wrapping
    const std::string& desc = card.description;
    std::string line;
    for (char ch : desc) {
        line += ch;
        if (line.size() > 28 && ch == ' ') {
            DrawText(line.c_str(), static_cast<int>(bounds.x + 12.0f), descY, 12, Colors::TextWhite);
            descY += 18;
            line.clear();
        }
    }
    if (!line.empty()) {
        DrawText(line.c_str(), static_cast<int>(bounds.x + 12.0f), descY, 12, Colors::TextWhite);
    }
}

void Renderer::RenderGameHUD(
    const IGrid& grid,
    const ActivePiece& piece,
    const PieceSpawner& spawner,
    const RunManager& runManager,
    const ParticleSystem& particles,
    const ScreenEffects& effects,
    bool showDebugPhysics
) const {
    // 1. Draw Title Header
    DrawText("TETROSHIFT // MORPHOTETRIS", 32, 16, 22, Colors::TextAccent);
    DrawText("C++20 Roguelike Physics Engine", 450, 20, 14, Colors::TextDim);

    // 2. Left Column Panels
    const float leftX = 32.0f;
    DrawHoldPanel(spawner, { leftX, 56.0f, 220.0f, 130.0f });
    DrawRelicsPanel(runManager, { leftX, 200.0f, 220.0f, 340.0f });
    DrawControlsPanel({ leftX, 552.0f, 220.0f, 180.0f });

    // 3. Center Playfield (with screen shake offset)
    Vector2 screenOffset = effects.GetScreenOffset();
    Vector2 gridOrigin = { PLAYFIELD_X + screenOffset.x, PLAYFIELD_Y + screenOffset.y };

    grid.Render(gridOrigin, CELL_SIZE, showDebugPhysics);
    piece.Render(grid, gridOrigin, CELL_SIZE, true, showDebugPhysics);

    // 4. Particles & Floating Combat Text
    particles.Render();

    // 5. Right Column Panels
    const float rightX = PLAYFIELD_X + (static_cast<float>(grid.GetWidth()) * CELL_SIZE) + 36.0f;
    DrawNextQueuePanel(spawner, { rightX, 56.0f, 220.0f, 290.0f });
    DrawStatsPanel(runManager, { rightX, 360.0f, 220.0f, 372.0f });

    // 6. Post-processing Overlays (Flash, Scanlines)
    effects.RenderPostProcessing(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void Renderer::DrawNowPlayingBanner(const char* title, const char* genre, float alpha) const {
    if (alpha <= 0.001f || !title) return;

    std::string text = std::string("♪ NOW PLAYING: ") + title;
    if (genre && genre[0] != '\0') {
        text += " [" + std::string(genre) + "]";
    }

    int textW = MeasureText(text.c_str(), 12);
    float boxW = static_cast<float>(textW + 36);
    float boxH = 28.0f;
    Rectangle pill = { (WINDOW_WIDTH - boxW) * 0.5f, 14.0f, boxW, boxH };

    DrawRectangleRounded(pill, 0.4f, 4, Fade(Colors::BgPanel, alpha * 0.90f));
    DrawRectangleLinesEx(pill, 1.2f, Fade(Colors::PieceI, alpha * 0.85f));
    DrawText(text.c_str(), static_cast<int>(pill.x + 18.0f), static_cast<int>(pill.y + 7.0f), 12, Fade(Colors::PieceI, alpha));
}

} // namespace TetroShift
