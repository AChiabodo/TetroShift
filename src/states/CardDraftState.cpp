#include "CardDraftState.hpp"
#include "PlayState.hpp"
#include "core/GameApp.hpp"
#include "core/Constants.hpp"
#include "render/Renderer.hpp"
#include <cmath>

namespace TetroShift {

CardDraftState::CardDraftState(int floorNumber) : m_floorNumber(floorNumber) {}

void CardDraftState::OnEnter(GameApp& app) {
    m_animTimer = 0.0f;
    m_selectedIndex = 0;
    m_hoveredIndex = -1;

    // Generate choices
    m_choices = app.GetCardDatabase().GenerateDraftChoices(3, app.GetPlayStateInventory(), m_floorNumber);
}

void CardDraftState::OnExit(GameApp& /*app*/) {}

void CardDraftState::Update(GameApp& /*app*/, float dt) {
    m_animTimer += dt;
}

void CardDraftState::SelectCard(GameApp& app, size_t index) {
    if (index >= m_choices.size()) return;

    app.GetSoundSynth().PlayCardSelect();

    // Add selected card to inventory and advance floor
    app.ApplyDraftCard(m_choices[index]);

    // Pop draft overlay to resume play
    app.GetStateManager().PopOverlay(app);
}

void CardDraftState::Reroll(GameApp& app) {
    if (app.UseRerollToken()) {
        app.GetSoundSynth().PlayMove();
        m_choices = app.GetCardDatabase().GenerateDraftChoices(3, app.GetPlayStateInventory(), m_floorNumber);
        m_selectedIndex = 0;
    }
}

void CardDraftState::HandleInput(GameApp& app) {
    // Keyboard navigation
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_choices.size())) % static_cast<int>(m_choices.size());
        app.GetSoundSynth().PlayMove();
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_choices.size());
        app.GetSoundSynth().PlayMove();
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        SelectCard(app, m_selectedIndex);
    }

    // Direct key shortcuts [1, 2, 3]
    if (IsKeyPressed(KEY_ONE) && m_choices.size() >= 1) SelectCard(app, 0);
    if (IsKeyPressed(KEY_TWO) && m_choices.size() >= 2) SelectCard(app, 1);
    if (IsKeyPressed(KEY_THREE) && m_choices.size() >= 3) SelectCard(app, 2);

    // Reroll shortcut [R]
    if (IsKeyPressed(KEY_R)) {
        Reroll(app);
    }

    // Mouse selection
    Vector2 mousePos = GetMousePosition();
    const float cardWidth = 260.0f;
    const float cardHeight = 360.0f;
    const float cardSpacing = 40.0f;
    const float totalWidth = (cardWidth * static_cast<float>(m_choices.size())) + (cardSpacing * static_cast<float>(m_choices.size() - 1));
    const float startX = (WINDOW_WIDTH - totalWidth) * 0.5f;
    const float startY = (WINDOW_HEIGHT - cardHeight) * 0.5f + 20.0f;

    m_hoveredIndex = -1;
    for (size_t i = 0; i < m_choices.size(); ++i) {
        Rectangle cardRect = { startX + static_cast<float>(i) * (cardWidth + cardSpacing), startY, cardWidth, cardHeight };
        if (CheckCollisionPointRec(mousePos, cardRect)) {
            m_hoveredIndex = static_cast<int>(i);
            m_selectedIndex = static_cast<int>(i);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                SelectCard(app, i);
                return;
            }
        }
    }

    // Reroll button mouse check
    Rectangle rerollBtn = { WINDOW_WIDTH * 0.5f - 80.0f, startY + cardHeight + 35.0f, 160.0f, 40.0f };
    if (CheckCollisionPointRec(mousePos, rerollBtn)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Reroll(app);
        }
    }
}

void CardDraftState::Render(GameApp& app) {
    // Dim background overlay
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.75f));

    // Title banner
    const char* header = "FLOOR COMPLETED // CHOOSE YOUR UPGRADE";
    int hw = MeasureText(header, 28);
    float glow = (std::sin(m_animTimer * 4.0f) + 1.0f) * 0.5f;
    DrawText(header, (WINDOW_WIDTH - hw) / 2, 85, 28, ColorAlphaBlend(GOLD, WHITE, Fade(WHITE, glow * 0.35f)));

    std::string sub = "ADVANCING TO FLOOR " + std::to_string(m_floorNumber + 1) + "  *  SELECT 1 OF 3 REWARDS";
    int sw = MeasureText(sub.c_str(), 14);
    DrawText(sub.c_str(), (WINDOW_WIDTH - sw) / 2, 125, 14, Colors::TextDim);

    // Cards layout
    const float cardWidth = 260.0f;
    const float cardHeight = 360.0f;
    const float cardSpacing = 40.0f;
    const float totalWidth = (cardWidth * static_cast<float>(m_choices.size())) + (cardSpacing * static_cast<float>(m_choices.size() - 1));
    const float startX = (WINDOW_WIDTH - totalWidth) * 0.5f;
    const float startY = (WINDOW_HEIGHT - cardHeight) * 0.5f + 20.0f;

    Renderer renderer;
    for (size_t i = 0; i < m_choices.size(); ++i) {
        float floatOffset = (static_cast<int>(i) == m_selectedIndex) ? std::sin(m_animTimer * 5.0f) * 4.0f - 8.0f : 0.0f;
        Rectangle cardRect = {
            startX + static_cast<float>(i) * (cardWidth + cardSpacing),
            startY + floatOffset,
            cardWidth,
            cardHeight
        };

        bool isSelected = (static_cast<int>(i) == m_selectedIndex);
        bool isHovered = (static_cast<int>(i) == m_hoveredIndex);

        renderer.DrawCardUI(m_choices[i], cardRect, isSelected, isHovered);

        // Key shortcut badge
        std::string keyBadge = "KEY [" + std::to_string(i + 1) + "]";
        DrawRectangleRounded({ cardRect.x + cardRect.width * 0.5f - 40.0f, cardRect.y + cardRect.height - 30.0f, 80.0f, 20.0f }, 0.3f, 4, Colors::BgDark);
        DrawText(keyBadge.c_str(), static_cast<int>(cardRect.x + cardRect.width * 0.5f - 26.0f), static_cast<int>(cardRect.y + cardRect.height - 26.0f), 10, isSelected ? GOLD : Colors::TextDim);
    }

    // Reroll Button
    Rectangle rerollBtn = { WINDOW_WIDTH * 0.5f - 80.0f, startY + cardHeight + 35.0f, 160.0f, 40.0f };
    int remainingRerolls = app.GetRemainingRerolls();
    bool canReroll = (remainingRerolls > 0);

    DrawRectangleRounded(rerollBtn, 0.2f, 4, canReroll ? Colors::BgPanel : Fade(Colors::BgPanel, 0.4f));
    DrawRectangleLinesEx(rerollBtn, 1.5f, canReroll ? Colors::TextAccent : Colors::BgPanelBorder);

    std::string rerollText = "REROLL [R] (" + std::to_string(remainingRerolls) + ")";
    int rw = MeasureText(rerollText.c_str(), 13);
    DrawText(rerollText.c_str(), static_cast<int>(rerollBtn.x + (rerollBtn.width - rw) * 0.5f), static_cast<int>(rerollBtn.y + 13.0f), 13, canReroll ? Colors::TextAccent : Colors::TextDim);
}

} // namespace TetroShift
