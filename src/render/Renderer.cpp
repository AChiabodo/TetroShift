#include "Renderer.hpp"
#include "core/Constants.hpp"
#include "roguelike/Card.hpp"
#include "roguelike/HazardManager.hpp"
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

void Renderer::DrawStatsPanel(const RunManager& runManager, Rectangle bounds, GameMode gameMode, int marathonLevel) const {
    DrawPanelFrame(bounds, (gameMode == GameMode::Marathon) ? "MARATHON STATS" : "RUN STATS");

    int textX = static_cast<int>(bounds.x + 16.0f);
    int textY = static_cast<int>(bounds.y + 22.0f);
    int spacing = 26;

    // Score
    DrawText("SCORE", textX, textY, 12, Colors::TextDim);
    std::string scoreStr = std::to_string(runManager.GetScore());
    DrawText(scoreStr.c_str(), textX, textY + 13, 20, Colors::TextWhite);
    textY += spacing + 12;

    if (gameMode == GameMode::Marathon) {
        // Marathon Level
        DrawText("CURRENT LEVEL", textX, textY, 12, Colors::TextDim);
        std::string lvlStr = "LEVEL " + std::to_string(marathonLevel);
        DrawText(lvlStr.c_str(), textX, textY + 13, 18, Colors::TextAccent);
        textY += spacing + 10;

        // Lines to next level
        int linesToNext = 10 - (runManager.GetLinesTotal() % 10);
        DrawText("LINES TO NEXT LVL", textX, textY, 12, Colors::TextDim);
        std::string progStr = std::to_string(linesToNext) + " LINES REMAINING";
        DrawText(progStr.c_str(), textX, textY + 13, 12, Colors::TextWhite);

        // Progress Bar (0..10 lines)
        float progress = static_cast<float>(10 - linesToNext) / 10.0f;
        Rectangle barBg = { static_cast<float>(textX), static_cast<float>(textY + 28), bounds.width - 32.0f, 10.0f };
        DrawRectangleRounded(barBg, 0.4f, 4, Colors::GridBg);
        if (progress > 0.0f) {
            Rectangle barFill = { barBg.x, barBg.y, barBg.width * progress, barBg.height };
            DrawRectangleRounded(barFill, 0.4f, 4, Colors::TextGreen);
        }
        textY += spacing + 18;
    } else {
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
    }

    // Coins & Score Multiplier
    DrawText("COINS", textX, textY, 12, Colors::TextDim);
    std::string coinsStr = "$" + std::to_string(runManager.GetInventory().GetCoins());
    DrawText(coinsStr.c_str(), textX, textY + 13, 18, Colors::TextGold);

    DrawText("MULT", textX + 110, textY, 12, Colors::TextDim);
    char multBuffer[16];
    snprintf(multBuffer, sizeof(multBuffer), "x%.2f", runManager.GetScoreMultiplier());
    DrawText(multBuffer, textX + 110, textY + 13, 18, Colors::TextGreen);
    textY += spacing + 12;

    // Combo & Back-to-Back Status
    if (runManager.GetB2BStreak() > 0) {
        Rectangle b2bBadge = { static_cast<float>(textX), static_cast<float>(textY), 95.0f, 24.0f };
        DrawRectangleRounded(b2bBadge, 0.25f, 4, Fade(Colors::PieceGold, 0.25f));
        DrawRectangleLinesEx(b2bBadge, 1.2f, Colors::PieceGold);
        std::string b2bStr = "B2B x" + std::to_string(runManager.GetB2BStreak());
        DrawText(b2bStr.c_str(), static_cast<int>(b2bBadge.x + 12.0f), static_cast<int>(b2bBadge.y + 5.0f), 12, Colors::TextGold);
    }

    if (runManager.GetCombo() > 0) {
        float comboX = (runManager.GetB2BStreak() > 0) ? (static_cast<float>(textX) + 102.0f) : static_cast<float>(textX);
        Rectangle comboBadge = { comboX, static_cast<float>(textY), (runManager.GetB2BStreak() > 0) ? 80.0f : 180.0f, 24.0f };
        DrawRectangleRounded(comboBadge, 0.25f, 4, Fade(Colors::PieceBomb, 0.25f));
        DrawRectangleLinesEx(comboBadge, 1.2f, Colors::PieceBomb);
        std::string comboStr = "🔥 x" + std::to_string(runManager.GetCombo());
        DrawText(comboStr.c_str(), static_cast<int>(comboBadge.x + 10.0f), static_cast<int>(comboBadge.y + 5.0f), 12, Colors::PieceBomb);
    }
}

void Renderer::DrawMarathonPanel(int level, float fallInterval, int linesTotal, Rectangle bounds) const {
    DrawPanelFrame(bounds, "MARATHON SPEED & LEVEL");

    int tx = static_cast<int>(bounds.x + 14.0f);
    int ty = static_cast<int>(bounds.y + 24.0f);

    DrawText("MARATHON TARGET: 150 LINES", tx, ty, 11, Colors::TextAccent); ty += 20;
    DrawText("SPEED DIFFICULTY:", tx, ty, 11, Colors::TextDim); ty += 16;

    char spdBuf[64];
    snprintf(spdBuf, sizeof(spdBuf), "%.3f SEC / ROW", fallInterval);
    DrawText(spdBuf, tx, ty, 15, Colors::TextWhite); ty += 26;

    // Progression bar across 15 levels
    float totalProgress = static_cast<float>(std::min(150, linesTotal)) / 150.0f;
    DrawText("OVERALL RUN COMPLETION:", tx, ty, 11, Colors::TextDim); ty += 16;
    Rectangle progBar = { static_cast<float>(tx), static_cast<float>(ty), bounds.width - 28.0f, 12.0f };
    DrawRectangleRounded(progBar, 0.4f, 4, Colors::GridBg);
    if (totalProgress > 0.0f) {
        Rectangle fill = { progBar.x, progBar.y, progBar.width * totalProgress, progBar.height };
        DrawRectangleRounded(fill, 0.4f, 4, (level >= 15) ? GOLD : Colors::PieceI);
    }
    ty += 24;

    std::string linesDone = std::to_string(linesTotal) + " / 150 TOTAL LINES";
    DrawText(linesDone.c_str(), tx, ty, 12, Colors::TextWhite); ty += 24;

    DrawLine(static_cast<int>(bounds.x + 10.0f), ty, static_cast<int>(bounds.x + bounds.width - 10.0f), ty, Colors::BgPanelBorder);
    ty += 14;

    DrawText("SRS STANDARDS ACTIVE", tx, ty, 11, Colors::TextGreen); ty += 16;
    DrawText("* Pure Arcade Mechanics", tx, ty, 10, Colors::TextDim); ty += 14;
    DrawText("* No Roguelike Mutators", tx, ty, 10, Colors::TextDim); ty += 14;
    DrawText("* Dedicated Leaderboards", tx, ty, 10, Colors::TextDim);
}

void Renderer::DrawSandboxToolbox(bool zeroGravity, float elasticity, int selectedPiece, int selectedMino, Rectangle bounds) const {
    DrawPanelFrame(bounds, "SANDBOX TOOLBOX");

    int tx = static_cast<int>(bounds.x + 14.0f);
    int ty = static_cast<int>(bounds.y + 22.0f);

    DrawText("PIECE SPAWNER [1..7]:", tx, ty, 11, Colors::TextAccent); ty += 16;
    const char* pieces[7] = { "I", "J", "L", "O", "S", "T", "Z" };
    Color pCols[7] = { Colors::PieceI, Colors::PieceJ, Colors::PieceL, Colors::PieceO, Colors::PieceS, Colors::PieceT, Colors::PieceZ };
    
    for (int i = 0; i < 7; ++i) {
        Rectangle pBtn = { static_cast<float>(tx + i * 26), static_cast<float>(ty), 22.0f, 22.0f };
        bool isSel = (selectedPiece == i);
        DrawRectangleRounded(pBtn, 0.2f, 4, isSel ? Fade(pCols[i], 0.4f) : Colors::BgDark);
        DrawRectangleLinesEx(pBtn, isSel ? 1.5f : 1.0f, pCols[i]);
        DrawText(pieces[i], static_cast<int>(pBtn.x + 7.0f), static_cast<int>(pBtn.y + 4.0f), 12, pCols[i]);
    }
    ty += 32;

    // Mino Type Selector
    DrawText("MINO TYPE [F6]:", tx, ty, 11, Colors::TextAccent); ty += 16;
    const char* minoNames[5] = { "NORMAL", "SAND", "BOMB", "GOLD", "JELLY" };
    Color minoCols[5] = { Colors::TextWhite, Colors::PieceSand, Colors::PieceBomb, Colors::PieceGold, Colors::PieceJelly };
    
    Rectangle mBtn = { static_cast<float>(tx), static_cast<float>(ty), bounds.width - 28.0f, 24.0f };
    DrawRectangleRounded(mBtn, 0.2f, 4, Fade(minoCols[selectedMino % 5], 0.2f));
    DrawRectangleLinesEx(mBtn, 1.2f, minoCols[selectedMino % 5]);
    std::string mStr = "< " + std::string(minoNames[selectedMino % 5]) + " MINO >";
    int mtw = MeasureText(mStr.c_str(), 11);
    DrawText(mStr.c_str(), static_cast<int>(mBtn.x + (mBtn.width - mtw) * 0.5f), static_cast<int>(mBtn.y + 6.0f), 11, minoCols[selectedMino % 5]);
    ty += 34;

    // Gravity Mode Toggle
    DrawText("GRAVITY MODE [F7]:", tx, ty, 11, Colors::TextAccent); ty += 16;
    Rectangle gBtn = { static_cast<float>(tx), static_cast<float>(ty), bounds.width - 28.0f, 24.0f };
    DrawRectangleRounded(gBtn, 0.2f, 4, zeroGravity ? Fade(RED, 0.25f) : Fade(Colors::TextGreen, 0.25f));
    DrawRectangleLinesEx(gBtn, 1.2f, zeroGravity ? RED : Colors::TextGreen);
    const char* gStr = zeroGravity ? "FROZEN (0G TRAINING)" : "NORMAL GRAVITY";
    int gtw = MeasureText(gStr, 11);
    DrawText(gStr, static_cast<int>(gBtn.x + (gBtn.width - gtw) * 0.5f), static_cast<int>(gBtn.y + 6.0f), 11, zeroGravity ? RED : Colors::TextGreen);
    ty += 34;

    // Spring Elasticity
    DrawText("SOFT-BODY SPRING [F8/F9]:", tx, ty, 11, Colors::TextAccent); ty += 16;
    char elBuf[32];
    snprintf(elBuf, sizeof(elBuf), "ELASTICITY: %.1fx", elasticity);
    DrawText(elBuf, tx, ty, 12, Colors::TextWhite); ty += 22;

    DrawLine(static_cast<int>(bounds.x + 10.0f), ty, static_cast<int>(bounds.x + bounds.width - 10.0f), ty, Colors::BgPanelBorder);
    ty += 14;

    DrawText("[F10] : Wipe / Clear Grid", tx, ty, 11, RED); ty += 16;
    DrawText("[F11] : Drop Solid Garbage Row", tx, ty, 11, Colors::PieceBomb);
}

void Renderer::DrawRelicsPanel(const RunManager& runManager, Rectangle bounds) const {
    DrawPanelFrame(bounds, "ACTIVE ABILITIES & RELICS");

    const auto& actives = runManager.GetInventory().GetActives();
    const auto& passives = runManager.GetInventory().GetPassives();
    int shields = runManager.GetInventory().GetShieldCount();

    float currY = bounds.y + 20.0f;

    // Deflector Shield status badge
    if (shields > 0) {
        Rectangle shieldBadge = { bounds.x + 10.0f, currY, bounds.width - 20.0f, 24.0f };
        DrawRectangleRounded(shieldBadge, 0.25f, 4, Fade(Colors::PieceI, 0.25f));
        DrawRectangleLinesEx(shieldBadge, 1.2f, Colors::PieceI);
        std::string sText = "🛡 DEFLECTOR SHIELD (" + std::to_string(shields) + "x)";
        DrawText(sText.c_str(), static_cast<int>(shieldBadge.x + 10.0f), static_cast<int>(shieldBadge.y + 5.0f), 11, Colors::PieceI);
        currY += 30.0f;
    }

    // Active Abilities section
    if (!actives.empty()) {
        DrawText("ACTIVE ABILITIES [KEY 1 / 2]:", static_cast<int>(bounds.x + 12.0f), static_cast<int>(currY), 11, Colors::TextDim);
        currY += 16.0f;

        for (size_t i = 0; i < actives.size() && i < 2; ++i) {
            const auto& act = actives[i];
            Rectangle actRect = { bounds.x + 8.0f, currY, bounds.width - 16.0f, 42.0f };

            bool isReady = (act.currentCooldown == 0);
            Color borderCol = isReady ? Colors::PieceI : Colors::BgPanelBorder;
            Color bgCol = isReady ? Fade(Colors::PieceI, 0.20f) : Colors::BgDark;

            DrawRectangleRounded(actRect, 0.15f, 4, bgCol);
            DrawRectangleLinesEx(actRect, isReady ? 1.8f : 1.0f, isReady ? (GetTime() * 4.0f - std::floor(GetTime() * 4.0f) > 0.5f ? WHITE : Colors::PieceI) : borderCol);

            std::string slotKey = "[" + std::to_string(i + 1) + "] " + act.title;
            DrawText(slotKey.c_str(), static_cast<int>(actRect.x + 8.0f), static_cast<int>(actRect.y + 6.0f), 12, isReady ? Colors::TextWhite : Colors::TextDim);

            if (isReady) {
                std::string readyTxt = "READY! [PREMI " + std::to_string(i + 1) + "]";
                DrawText(readyTxt.c_str(), static_cast<int>(actRect.x + 8.0f), static_cast<int>(actRect.y + 24.0f), 11, Colors::TextGreen);
            } else {
                float cdProgress = 1.0f - (static_cast<float>(act.currentCooldown) / static_cast<float>(std::max(1, act.maxCooldown)));
                Rectangle cdBarBg = { actRect.x + 8.0f, actRect.y + 24.0f, actRect.width - 80.0f, 8.0f };
                DrawRectangleRounded(cdBarBg, 0.4f, 4, Colors::GridBg);
                Rectangle cdBarFill = { cdBarBg.x, cdBarBg.y, cdBarBg.width * cdProgress, cdBarBg.height };
                DrawRectangleRounded(cdBarFill, 0.4f, 4, Colors::PieceBomb);

                std::string cdText = std::to_string(act.currentCooldown) + "L";
                DrawText(cdText.c_str(), static_cast<int>(actRect.x + actRect.width - 65.0f), static_cast<int>(actRect.y + 22.0f), 11, Colors::TextDanger);
            }
            currY += 48.0f;
        }
        currY += 4.0f;
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

void Renderer::DrawControlsPanel(Rectangle bounds, GameMode gameMode) const {
    DrawPanelFrame(bounds, "CONTROLS");

    int tx = static_cast<int>(bounds.x + 12.0f);
    int ty = static_cast<int>(bounds.y + 18.0f);

    DrawText("LEFT / RIGHT / DOWN : Move & Soft Drop", tx, ty, 10, Colors::TextDim); ty += 15;
    DrawText("UP / X : Rotate CW  |  Z : Rotate CCW", tx, ty, 10, Colors::TextDim); ty += 15;
    DrawText("SPACE : Hard Drop   |  C : Hold", tx, ty, 10, Colors::TextDim); ty += 15;
    
    if (gameMode == GameMode::Sandbox) {
        DrawText("1..7 : Spawn Piece  |  F6 : Mino Type", tx, ty, 10, Colors::PieceSand); ty += 15;
        DrawText("F7 : Toggle 0G      |  F8/F9 : Spring", tx, ty, 10, Colors::TextAccent);
    } else if (gameMode == GameMode::Marathon) {
        DrawText("P : Pause Game      |  F1 : Physics Mesh", tx, ty, 10, Colors::TextDim); ty += 15;
        DrawText("MARATHON SPEED PROGRESSION ACTIVE", tx, ty, 10, Colors::TextAccent);
    } else {
        DrawText("1 / 2 : Use Active  |  P : Pause", tx, ty, 10, Colors::TextDim); ty += 15;
        DrawText("F1: Physics  F2: Draft  F3: Clear  F5: Reset", tx, ty, 10, Colors::TextAccent);
    }
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
    bool showDebugPhysics,
    const class HazardManager* hazardManager,
    GameMode gameMode,
    int marathonLevel,
    float marathonFallInterval,
    bool sandboxZeroGravity,
    float sandboxElasticity,
    int sandboxPiece,
    int sandboxMino,
    const std::string& dailyDate
) const {
    // 1. Draw Title Header by Mode
    if (gameMode == GameMode::Marathon) {
        std::string t = "TETROSHIFT // MATRIX MARATHON (LVL " + std::to_string(marathonLevel) + ")";
        DrawText(t.c_str(), 32, 16, 22, Colors::TextAccent);
        DrawText("Standard Classic SRS Arcade Mode", 500, 20, 14, Colors::TextDim);
    } else if (gameMode == GameMode::DailyProtocol) {
        std::string t = "TETROSHIFT // DAILY PROTOCOL [" + dailyDate + "]";
        DrawText(t.c_str(), 32, 16, 22, Colors::TextGold);
        DrawText("Synchronized Daily Global Challenge", 500, 20, 14, Colors::TextDim);
    } else if (gameMode == GameMode::Sandbox) {
        DrawText("TETROSHIFT // TRAINING & PHYSICS SANDBOX", 32, 16, 22, Colors::PieceSand);
        DrawText("Interactive Mino & Mechanics Testing Laboratory", 500, 20, 14, Colors::TextDim);
    } else {
        DrawText("TETROSHIFT // ROGUELIKE CAMPAIGN", 32, 16, 22, Colors::TextAccent);
        DrawText("C++20 Roguelike Physics Engine", 450, 20, 14, Colors::TextDim);
    }

    // 2. Left Column Panels
    const float leftX = 32.0f;
    DrawHoldPanel(spawner, { leftX, 56.0f, 220.0f, 130.0f });

    if (gameMode == GameMode::Marathon) {
        DrawMarathonPanel(marathonLevel, marathonFallInterval, runManager.GetLinesTotal(), { leftX, 200.0f, 220.0f, 340.0f });
    } else if (gameMode == GameMode::Sandbox) {
        DrawSandboxToolbox(sandboxZeroGravity, sandboxElasticity, sandboxPiece, sandboxMino, { leftX, 200.0f, 220.0f, 340.0f });
    } else {
        DrawRelicsPanel(runManager, { leftX, 200.0f, 220.0f, 340.0f });
    }

    DrawControlsPanel({ leftX, 552.0f, 220.0f, 180.0f }, gameMode);

    // 3. Center Playfield (with screen shake offset)
    Vector2 screenOffset = effects.GetScreenOffset();
    Vector2 gridOrigin = { PLAYFIELD_X + screenOffset.x, PLAYFIELD_Y + screenOffset.y };

    // Hazard warning banner above matrix
    if (hazardManager && hazardManager->HasActiveHazard() && gameMode == GameMode::Roguelike) {
        std::string hText = hazardManager->GetActiveStatusText();
        Color hCol = hazardManager->GetConfig().themeColor;
        bool isPulsing = hazardManager->IsPulseActive();

        Rectangle hBanner = { gridOrigin.x - 20.0f, gridOrigin.y - 34.0f, (static_cast<float>(grid.GetWidth()) * CELL_SIZE) + 40.0f, 26.0f };
        DrawRectangleRounded(hBanner, 0.3f, 4, isPulsing ? Fade(hCol, 0.35f) : Colors::BgPanel);
        DrawRectangleLinesEx(hBanner, isPulsing ? 2.0f : 1.0f, isPulsing ? WHITE : hCol);
        int htw = MeasureText(hText.c_str(), 11);
        DrawText(hText.c_str(), static_cast<int>(hBanner.x + (hBanner.width - htw) * 0.5f), static_cast<int>(hBanner.y + 7.0f), 11, isPulsing ? WHITE : hCol);
    }

    grid.Render(gridOrigin, CELL_SIZE, showDebugPhysics);

    // Deflector shield energy aura around matrix
    if (runManager.GetInventory().GetShieldCount() > 0 && gameMode != GameMode::Marathon) {
        float gridW = static_cast<float>(grid.GetWidth()) * CELL_SIZE;
        float gridH = static_cast<float>(grid.GetHeight()) * CELL_SIZE;
        DrawRectangleLinesEx({ gridOrigin.x - 4.0f, gridOrigin.y - 4.0f, gridW + 8.0f, gridH + 8.0f }, 2.0f, Fade(Colors::PieceI, 0.7f));
    }

    piece.Render(grid, gridOrigin, CELL_SIZE, true, showDebugPhysics);

    // 4. Particles & Floating Combat Text
    particles.Render();

    // 5. Right Column Panels
    const float rightX = PLAYFIELD_X + (static_cast<float>(grid.GetWidth()) * CELL_SIZE) + 36.0f;
    DrawNextQueuePanel(spawner, { rightX, 56.0f, 220.0f, 290.0f });
    DrawStatsPanel(runManager, { rightX, 360.0f, 220.0f, 372.0f }, gameMode, marathonLevel);

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
