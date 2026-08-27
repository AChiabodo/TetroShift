#include "TitleState.hpp"
#include "PlayState.hpp"
#include "core/GameApp.hpp"
#include "core/Constants.hpp"
#include <cmath>

namespace TetroShift {

void TitleState::OnEnter(GameApp& /*app*/) {
    m_animTimer = 0.0f;
    m_selectedOption = 0;
}

void TitleState::OnExit(GameApp& /*app*/) {}

void TitleState::Update(GameApp& /*app*/, float dt) {
    m_animTimer += dt;
}

void TitleState::HandleInput(GameApp& app) {
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
        } else if (m_selectedOption == 1) {
            // Exit game
            CloseWindow();
        }
    }

    // Mouse support
    Vector2 mousePos = GetMousePosition();
    Rectangle btnPlay = { WINDOW_WIDTH * 0.5f - 140.0f, 420.0f, 280.0f, 50.0f };
    Rectangle btnQuit = { WINDOW_WIDTH * 0.5f - 140.0f, 490.0f, 280.0f, 50.0f };

    if (CheckCollisionPointRec(mousePos, btnPlay)) {
        m_selectedOption = 0;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            app.GetSoundSynth().PlayCardSelect();
            app.GetStateManager().SetState(app, std::make_unique<PlayState>());
        }
    } else if (CheckCollisionPointRec(mousePos, btnQuit)) {
        m_selectedOption = 1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            CloseWindow();
        }
    }
}

void TitleState::Render(GameApp& /*app*/) {
    ClearBackground(Colors::BgDark);

    // Decorative floating neon background blocks
    for (int i = 0; i < 8; ++i) {
        float x = 120.0f + static_cast<float>(i) * 150.0f;
        float y = 200.0f + std::sin(m_animTimer * 1.5f + static_cast<float>(i)) * 40.0f;
        DrawRectangleRounded({ x, y, 40.0f, 40.0f }, 0.2f, 4, Fade(Colors::PieceI, 0.08f));
        DrawRectangleLinesEx({ x, y, 40.0f, 40.0f }, 1.5f, Fade(Colors::PieceT, 0.15f));
    }

    // Glowing Title Header
    const char* title = "TETROSHIFT";
    const int titleSize = 72;
    int titleWidth = MeasureText(title, titleSize);
    float glow = (std::sin(m_animTimer * 3.0f) + 1.0f) * 0.5f;

    DrawText(title, (WINDOW_WIDTH - titleWidth) / 2, 160, titleSize, ColorAlphaBlend(Colors::PieceI, WHITE, Fade(WHITE, glow * 0.4f)));

    const char* subtitle = "MORPHOTETRIS // ROGUELIKE PHYSICS ENGINE";
    int subWidth = MeasureText(subtitle, 18);
    DrawText(subtitle, (WINDOW_WIDTH - subWidth) / 2, 245, 18, Colors::TextDim);

    const char* featureText = "C++20  *  Raylib 5.0  *  Soft-Body Physics  *  Card Draft System";
    int featWidth = MeasureText(featureText, 14);
    DrawText(featureText, (WINDOW_WIDTH - featWidth) / 2, 290, 14, Colors::TextAccent);

    // Buttons
    Rectangle btnPlay = { WINDOW_WIDTH * 0.5f - 140.0f, 420.0f, 280.0f, 50.0f };
    Rectangle btnQuit = { WINDOW_WIDTH * 0.5f - 140.0f, 490.0f, 280.0f, 50.0f };

    // Play Button
    DrawRectangleRounded(btnPlay, 0.2f, 6, (m_selectedOption == 0) ? Fade(Colors::TextGreen, 0.25f) : Colors::BgPanel);
    DrawRectangleLinesEx(btnPlay, 2.0f, (m_selectedOption == 0) ? Colors::TextGreen : Colors::BgPanelBorder);
    const char* playText = "START NEW RUN";
    int playWidth = MeasureText(playText, 20);
    DrawText(playText, static_cast<int>(btnPlay.x + (btnPlay.width - playWidth) * 0.5f), static_cast<int>(btnPlay.y + 15.0f), 20, (m_selectedOption == 0) ? Colors::TextGreen : Colors::TextWhite);

    // Quit Button
    DrawRectangleRounded(btnQuit, 0.2f, 6, (m_selectedOption == 1) ? Fade(Colors::PieceBomb, 0.25f) : Colors::BgPanel);
    DrawRectangleLinesEx(btnQuit, 2.0f, (m_selectedOption == 1) ? Colors::PieceBomb : Colors::BgPanelBorder);
    const char* quitText = "EXIT GAME";
    int quitWidth = MeasureText(quitText, 20);
    DrawText(quitText, static_cast<int>(btnQuit.x + (btnQuit.width - quitWidth) * 0.5f), static_cast<int>(btnQuit.y + 15.0f), 20, (m_selectedOption == 1) ? Colors::PieceBomb : Colors::TextWhite);

    // Footer Hint
    const char* hint = "PRESS [ENTER / SPACE] TO START  *  USE ARROW KEYS OR MOUSE";
    int hintWidth = MeasureText(hint, 12);
    DrawText(hint, (WINDOW_WIDTH - hintWidth) / 2, 690, 12, Colors::TextDim);
}

} // namespace TetroShift
