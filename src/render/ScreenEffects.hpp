#pragma once
#include <raylib.h>
#include <string>

namespace TetroShift {

class ScreenEffects {
public:
    ScreenEffects();
    ~ScreenEffects();

    // Non-copyable, movable
    ScreenEffects(const ScreenEffects&) = delete;
    ScreenEffects& operator=(const ScreenEffects&) = delete;
    ScreenEffects(ScreenEffects&& other) noexcept;
    ScreenEffects& operator=(ScreenEffects&& other) noexcept;

    void Initialize(int screenWidth, int screenHeight);
    void Shutdown();

    void Reset();
    void Update(float dt);

    void AddTrauma(float amount);
    void TriggerFlash(Color color, float duration = 0.3f);

    // Toggles & Settings
    void ToggleScanlines() noexcept { m_scanlinesEnabled = !m_scanlinesEnabled; }
    void ToggleCrtCurvature() noexcept { m_crtCurvatureEnabled = !m_crtCurvatureEnabled; }
    void ToggleBloom() noexcept { m_bloomEnabled = !m_bloomEnabled; }

    void SetScanlinesEnabled(bool enabled) noexcept { m_scanlinesEnabled = enabled; }
    void SetCrtCurvatureEnabled(bool enabled) noexcept { m_crtCurvatureEnabled = enabled; }
    void SetBloomEnabled(bool enabled) noexcept { m_bloomEnabled = enabled; }

    [[nodiscard]] bool IsScanlinesEnabled() const noexcept { return m_scanlinesEnabled; }
    [[nodiscard]] bool IsCrtCurvatureEnabled() const noexcept { return m_crtCurvatureEnabled; }
    [[nodiscard]] bool IsBloomEnabled() const noexcept { return m_bloomEnabled; }
    [[nodiscard]] float GetTrauma() const noexcept { return m_trauma; }

    [[nodiscard]] Vector2 GetScreenOffset() const noexcept;
    [[nodiscard]] float GetScreenRotation() const noexcept;

    // Full GPU Post-Processing Shader Pass
    void RenderPostProcessing(const RenderTexture2D& sceneTexture, int screenWidth, int screenHeight);
    
    // Legacy simple fallback overlay
    void RenderPostProcessing(int screenWidth, int screenHeight) const;

private:
    float m_trauma = 0.0f; // 0.0 to 1.0
    float m_flashTimer = 0.0f;
    float m_flashMaxDuration = 0.3f;
    Color m_flashColor = WHITE;

    bool m_scanlinesEnabled = true;
    bool m_crtCurvatureEnabled = false;
    bool m_bloomEnabled = true;

    float m_shaderTime = 0.0f;
    Shader m_postShader{};
    bool m_shaderLoaded = false;

    // Shader uniform locations
    int m_locResolution = -1;
    int m_locTime = -1;
    int m_locChromatic = -1;
    int m_locBloom = -1;
    int m_locCurvature = -1;
    int m_locScanline = -1;
    int m_locVignette = -1;

    void LoadPostShader();
};

} // namespace TetroShift
