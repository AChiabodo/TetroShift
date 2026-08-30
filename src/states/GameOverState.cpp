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

void GameOverState::Render(GameApp& /*app*/) {
    ClearBackground(Colors::BgDark);

    // Glowing Header
    const char* header = "RUN TERMINATED";
    int hw = MeasureText(header, 54);
    float glow = (std::sin(m_animTimer * 4.0f) + 1.0f) * 0.5f;
    DrawText(header, (WINDOW_WIDTH - hw) / 2, 130, 54, ColorAlphaBlend(Colors::PieceBomb, WHITE, Fade(WHITE, glow * 0.3f)));

    // Stats Panel Frame
    Rectangle statsRect = { WINDOW_WIDTH * 0.5f - 180.0f, 220.0f, 360.0f, 200.0f };
    DrawRectangleRounded(statsRect, 0.08f, 6, Colors::BgPanel);
    DrawRectangleLinesEx(statsRect, 2.0f, Colors::BgPanelBorder);

    int textX = static_cast<int>(statsRect.x + 30.0f);
    int textY = static_cast<int>(statsRect.y + 30.0f);

    // Final Score
    DrawText("FINAL SCORE", textX, textY, 13, Colors::TextDim);
    std::string scoreStr = std::to_string(m_finalScore);
    DrawText(scoreStr.c_str(), textX, textY + 16, 26, GOLD);
    textY += 55;

    // Floor Reached
    DrawText("FLOOR REACHED", textX, textY, 13, Colors::TextDim);
    std::string floorStr = "FLOOR " + std::to_string(m_finalFloor);
    DrawText(floorStr.c_str(), textX, textY + 16, 22, Colors::TextAccent);
    textY += 50;

    // Total Lines
    DrawText("TOTAL LINES CLEARED", textX, textY, 13, Colors::TextDim);
    std::string linesStr = std::to_string(m_finalLines) + " LINES";
    DrawText(linesStr.c_str(), textX, textY + 16, 20, Colors::TextWhite);

    // Buttons
    Rectangle btnRestart = { WINDOW_WIDTH * 0.5f - 140.0f, 460.0f, 280.0f, 48.0f };
    Rectangle btnMenu = { WINDOW_WIDTH * 0.5f - 140.0f, 520.0f, 280.0f, 48.0f };

    DrawRectangleRounded(btnRestart, 0.2f, 4, (m_selectedOption == 0) ? Fade(Colors::TextGreen, 0.25f) : Colors::BgPanel);
    DrawRectangleLinesEx(btnRestart, 2.0f, (m_selectedOption == 0) ? Colors::TextGreen : Colors::BgPanelBorder);
    const char* rText = "PLAY AGAIN [ENTER]";
    int rw = MeasureText(rText, 18);
    DrawText(rText, static_cast<int>(btnRestart.x + (btnRestart.width - rw) * 0.5f), static_cast<int>(btnRestart.y + 14.0f), 18, (m_selectedOption == 0) ? Colors::TextGreen : Colors::TextWhite);

    DrawRectangleRounded(btnMenu, 0.2f, 4, (m_selectedOption == 1) ? Fade(Colors::TextAccent, 0.25f) : Colors::BgPanel);
    DrawRectangleLinesEx(btnMenu, 2.0f, (m_selectedOption == 1) ? Colors::TextAccent : Colors::BgPanelBorder);
    const char* mText = "MAIN MENU";
    int mw = MeasureText(mText, 18);
    DrawText(mText, static_cast<int>(btnMenu.x + (btnMenu.width - mw) * 0.5f), static_cast<int>(btnMenu.y + 14.0f), 18, (m_selectedOption == 1) ? Colors::TextAccent : Colors::TextWhite);
}

} // namespace TetroShift
