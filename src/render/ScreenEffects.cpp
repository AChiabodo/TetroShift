#include "ScreenEffects.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>

namespace TetroShift {

void ScreenEffects::Reset() {
    m_trauma = 0.0f;
    m_flashTimer = 0.0f;
    m_flashMaxDuration = 0.3f;
    m_flashColor = WHITE;
    m_scanlinesEnabled = false;
}

void ScreenEffects::Update(float dt) {
    // Decay trauma linearly
    if (m_trauma > 0.0f) {
        m_trauma = std::max(0.0f, m_trauma - dt * 1.5f);
    }

    // Decay flash
    if (m_flashTimer > 0.0f) {
        m_flashTimer = std::max(0.0f, m_flashTimer - dt);
    }
}

void ScreenEffects::AddTrauma(float amount) {
    m_trauma = std::min(1.0f, m_trauma + amount);
}

void ScreenEffects::TriggerFlash(Color color, float duration) {
    m_flashColor = color;
    m_flashMaxDuration = std::max(0.01f, duration);
    m_flashTimer = m_flashMaxDuration;
}

Vector2 ScreenEffects::GetScreenOffset() const noexcept {
    if (m_trauma <= 0.001f) {
        return { 0.0f, 0.0f };
    }

    const float shake = m_trauma * m_trauma; // Non-linear response
    const float maxOffset = 18.0f;

    const float offsetX = maxOffset * shake * ((static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f);
    const float offsetY = maxOffset * shake * ((static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f);

    return { offsetX, offsetY };
}

float ScreenEffects::GetScreenRotation() const noexcept {
    if (m_trauma <= 0.001f) return 0.0f;
    const float shake = m_trauma * m_trauma;
    const float maxAngle = 1.8f; // degrees
    return maxAngle * shake * ((static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f);
}

void ScreenEffects::RenderPostProcessing(int screenWidth, int screenHeight) const {
    // 1. Flash Overlay
    if (m_flashTimer > 0.0f) {
        float alpha = m_flashTimer / m_flashMaxDuration;
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(m_flashColor, alpha * 0.45f));
    }

    // 2. Scanlines / CRT effect (if toggled)
    if (m_scanlinesEnabled) {
        for (int y = 0; y < screenHeight; y += 4) {
            DrawRectangle(0, y, screenWidth, 1, Fade(BLACK, 0.25f));
        }
        // Subtle corner vignette
        DrawRectangleGradientEx({ 0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight) },
                                Fade(BLACK, 0.0f), Fade(BLACK, 0.0f), Fade(BLACK, 0.35f), Fade(BLACK, 0.35f));
    }
}

} // namespace TetroShift
