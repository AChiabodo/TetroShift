#pragma once
#include <raylib.h>

namespace TetroShift {

class ScreenEffects {
public:
    ScreenEffects() = default;

    void Reset();
    void Update(float dt);

    void AddTrauma(float amount);
    void TriggerFlash(Color color, float duration = 0.3f);
    void ToggleScanlines() noexcept { m_scanlinesEnabled = !m_scanlinesEnabled; }

    [[nodiscard]] Vector2 GetScreenOffset() const noexcept;
    [[nodiscard]] float GetScreenRotation() const noexcept;

    void RenderPostProcessing(int screenWidth, int screenHeight) const;

private:
    float m_trauma = 0.0f; // 0.0 to 1.0
    float m_flashTimer = 0.0f;
    float m_flashMaxDuration = 0.3f;
    Color m_flashColor = WHITE;
    bool m_scanlinesEnabled = false;
};

} // namespace TetroShift
