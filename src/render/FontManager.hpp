#pragma once
#include <raylib.h>
#include <string>
#include <vector>

namespace TetroShift {

class FontManager {
public:
    FontManager() = default;
    ~FontManager();

    // Non-copyable
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    void Initialize();
    void Shutdown();

    [[nodiscard]] Font GetTitleFont() const noexcept;
    [[nodiscard]] Font GetBodyFont() const noexcept;
    [[nodiscard]] Font GetMonoFont() const noexcept;

    // High-level text drawing helpers
    void DrawTitle(const char* text, Vector2 pos, float size, Color color, float spacing = 1.5f) const;
    void DrawBody(const char* text, Vector2 pos, float size, Color color, float spacing = 1.0f) const;
    void DrawMono(const char* text, Vector2 pos, float size, Color color, float spacing = 1.0f) const;

    // Glowing cyberpunk text with drop-shadow aura
    void DrawGlow(
        const Font& font,
        const char* text,
        Vector2 pos,
        float size,
        Color color,
        Color glowColor,
        float glowRadius = 2.0f,
        float spacing = 1.0f
    ) const;

    // Measurement helpers
    [[nodiscard]] Vector2 MeasureTitle(const char* text, float size, float spacing = 1.5f) const;
    [[nodiscard]] Vector2 MeasureBody(const char* text, float size, float spacing = 1.0f) const;
    [[nodiscard]] Vector2 MeasureMono(const char* text, float size, float spacing = 1.0f) const;

    [[nodiscard]] bool HasCustomFonts() const noexcept { return m_hasCustomFonts; }

private:
    Font m_fontTitle{};
    Font m_fontBody{};
    Font m_fontMono{};
    bool m_hasCustomFonts = false;
    bool m_initialized = false;
};

} // namespace TetroShift
