#include "GameOverState.hpp"
#include "PlayState.hpp"
#include "TitleState.hpp"
#include "core/GameApp.hpp"
#include "core/Constants.hpp"
#include <string>
#include <cmath>

namespace TetroShift {

GameOverState::GameOverState(int score, int floor, int lines)
    : m_finalScore(score), m_finalFloor(floor), m_finalLines(lines) {}

void GameOverState::OnEnter(GameApp& app) {
    m_animTimer = 0.0f;
    m_selectedOption = 0;
    app.GetMusicManager().PlayTrack(TrackId::GameOverTheme, true);
}

void GameOverState::OnExit(GameApp& /*app*/) {}

void GameOverState::Update(GameApp& /*app*/, float dt) {
    m_animTimer += dt;
}

void GameOverState::HandleInput(GameApp& app) {
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + 2) % 2;
        app.GetSoundSynth().PlayMove();
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % 2;
        app.GetSoundSynth().PlayMove();
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        app.GetSoundSynth().PlayCardSelect();
        if (m_selectedOption == 0) {
            app.GetStateManager().SetState(app, std::make_unique<PlayState>());
        } else {
            app.GetStateManager().SetState(app, std::make_unique<TitleState>());
        }
    }

    // Mouse support
    Vector2 mousePos = GetMousePosition();
    Rectangle btnRestart = { WINDOW_WIDTH * 0.5f - 140.0f, 460.0f, 280.0f, 48.0f };
    Rectangle btnMenu = { WINDOW_WIDTH * 0.5f - 140.0f, 520.0f, 280.0f, 48.0f };

    if (CheckCollisionPointRec(mousePos, btnRestart)) {
        m_selectedOption = 0;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            app.GetSoundSynth().PlayCardSelect();
            app.GetStateManager().SetState(app, std::make_unique<PlayState>());
        }
    } else if (CheckCollisionPointRec(mousePos, btnMenu)) {
        m_selectedOption = 1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            app.GetSoundSynth().PlayCardSelect();
            app.GetStateManager().SetState(app, std::make_unique<TitleState>());
        }
    }
}

void GameOverState::Render(GameApp& app) {
    ClearBackground(Colors::BgDark);

    const auto& fm = app.GetFontManager();

    // Glowing Header
    const char* header = "RUN TERMINATED";
    float glow = (std::sin(m_animTimer * 4.0f) + 1.0f) * 0.5f;

    if (fm.HasCustomFonts()) {
        Vector2 hw = fm.MeasureTitle(header, 54.0f);
        Vector2 pos = { (WINDOW_WIDTH - hw.x) * 0.5f, 120.0f };
        fm.DrawGlow(fm.GetTitleFont(), header, pos, 54.0f, Colors::PieceBomb, WHITE, 3.0f * glow);
    } else {
        int hw = MeasureText(header, 54);
        DrawText(header, (WINDOW_WIDTH - hw) / 2, 130, 54, ColorAlphaBlend(Colors::PieceBomb, WHITE, Fade(WHITE, glow * 0.3f)));
    }

    // Stats Panel Frame
    Rectangle statsRect = { WINDOW_WIDTH * 0.5f - 180.0f, 220.0f, 360.0f, 200.0f };
    DrawRectangleRounded(statsRect, 0.08f, 6, Colors::BgPanel);
    DrawRectangleLinesEx(statsRect, 2.0f, Colors::BgPanelBorder);

    float textX = statsRect.x + 30.0f;
    float textY = statsRect.y + 24.0f;

    // Final Score
    if (fm.HasCustomFonts()) {
        fm.DrawBody("FINAL SCORE", { textX, textY }, 13.0f, Colors::TextDim);
        std::string scoreStr = std::to_string(m_finalScore);
        fm.DrawTitle(scoreStr.c_str(), { textX, textY + 16.0f }, 26.0f, GOLD);
        textY += 55.0f;

        fm.DrawBody("FLOOR REACHED", { textX, textY }, 13.0f, Colors::TextDim);
        std::string floorStr = "FLOOR " + std::to_string(m_finalFloor);
        fm.DrawTitle(floorStr.c_str(), { textX, textY + 16.0f }, 22.0f, Colors::TextAccent);
        textY += 50.0f;

        fm.DrawBody("TOTAL LINES CLEARED", { textX, textY }, 13.0f, Colors::TextDim);
        std::string linesStr = std::to_string(m_finalLines) + " LINES";
        fm.DrawTitle(linesStr.c_str(), { textX, textY + 16.0f }, 20.0f, Colors::TextWhite);
    } else {
        DrawText("FINAL SCORE", static_cast<int>(textX), static_cast<int>(textY), 13, Colors::TextDim);
        std::string scoreStr = std::to_string(m_finalScore);
        DrawText(scoreStr.c_str(), static_cast<int>(textX), static_cast<int>(textY + 16), 26, GOLD);
        textY += 55;

        DrawText("FLOOR REACHED", static_cast<int>(textX), static_cast<int>(textY), 13, Colors::TextDim);
        std::string floorStr = "FLOOR " + std::to_string(m_finalFloor);
        DrawText(floorStr.c_str(), static_cast<int>(textX), static_cast<int>(textY + 16), 22, Colors::TextAccent);
        textY += 50;

        DrawText("TOTAL LINES CLEARED", static_cast<int>(textX), static_cast<int>(textY), 13, Colors::TextDim);
        std::string linesStr = std::to_string(m_finalLines) + " LINES";
        DrawText(linesStr.c_str(), static_cast<int>(textX), static_cast<int>(textY + 16), 20, Colors::TextWhite);
    }

    // Buttons
    Rectangle btnRestart = { WINDOW_WIDTH * 0.5f - 140.0f, 460.0f, 280.0f, 48.0f };
    Rectangle btnMenu = { WINDOW_WIDTH * 0.5f - 140.0f, 520.0f, 280.0f, 48.0f };

    DrawRectangleRounded(btnRestart, 0.2f, 4, (m_selectedOption == 0) ? Fade(Colors::TextGreen, 0.25f) : Colors::BgPanel);
    DrawRectangleLinesEx(btnRestart, 2.0f, (m_selectedOption == 0) ? Colors::TextGreen : Colors::BgPanelBorder);
    const char* rText = "PLAY AGAIN [ENTER]";
    if (fm.HasCustomFonts()) {
        Vector2 rw = fm.MeasureTitle(rText, 18.0f);
        fm.DrawTitle(rText, { btnRestart.x + (btnRestart.width - rw.x) * 0.5f, btnRestart.y + 14.0f }, 18.0f, (m_selectedOption == 0) ? Colors::TextGreen : Colors::TextWhite);
    } else {
        int rw = MeasureText(rText, 18);
        DrawText(rText, static_cast<int>(btnRestart.x + (btnRestart.width - rw) * 0.5f), static_cast<int>(btnRestart.y + 14.0f), 18, (m_selectedOption == 0) ? Colors::TextGreen : Colors::TextWhite);
    }

    DrawRectangleRounded(btnMenu, 0.2f, 4, (m_selectedOption == 1) ? Fade(Colors::TextAccent, 0.25f) : Colors::BgPanel);
    DrawRectangleLinesEx(btnMenu, 2.0f, (m_selectedOption == 1) ? Colors::TextAccent : Colors::BgPanelBorder);
    const char* mText = "MAIN MENU";
    if (fm.HasCustomFonts()) {
        Vector2 mw = fm.MeasureTitle(mText, 18.0f);
        fm.DrawTitle(mText, { btnMenu.x + (btnMenu.width - mw.x) * 0.5f, btnMenu.y + 14.0f }, 18.0f, (m_selectedOption == 1) ? Colors::TextAccent : Colors::TextWhite);
    } else {
        int mw = MeasureText(mText, 18);
        DrawText(mText, static_cast<int>(btnMenu.x + (btnMenu.width - mw) * 0.5f), static_cast<int>(btnMenu.y + 14.0f), 18, (m_selectedOption == 1) ? Colors::TextAccent : Colors::TextWhite);
    }
}

} // namespace TetroShift
