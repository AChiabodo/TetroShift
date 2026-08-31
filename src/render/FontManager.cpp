#include "FontManager.hpp"
#include <array>

namespace TetroShift {

FontManager::~FontManager() {
    Shutdown();
}

void FontManager::Initialize() {
    if (m_initialized) return;

    // Search candidates for Title / Display font
    const std::array<const char*, 6> titleCandidates = {
        "assets/fonts/Orbitron-Bold.ttf",
        "assets/fonts/orbitron_bold.ttf",
        "assets/fonts/orbitron.ttf",
        "assets/fonts/title.ttf",
        "assets/fonts/cyberpunk.ttf",
        "assets/fonts/font_title.ttf"
    };

    // Search candidates for Body / UI font
    const std::array<const char*, 6> bodyCandidates = {
        "assets/fonts/ShareTechMono-Regular.ttf",
        "assets/fonts/share_tech_mono.ttf",
        "assets/fonts/sharetechmono.ttf",
        "assets/fonts/body.ttf",
        "assets/fonts/roboto.ttf",
        "assets/fonts/font_body.ttf"
    };

    // Search candidates for Mono / Stats font
    const std::array<const char*, 6> monoCandidates = {
        "assets/fonts/PressStart2P-Regular.ttf",
        "assets/fonts/press_start_2p.ttf",
        "assets/fonts/pressstart2p.ttf",
        "assets/fonts/mono.ttf",
        "assets/fonts/ShareTechMono-Regular.ttf",
        "assets/fonts/font_mono.ttf"
    };

    // 1. Try loading Title Font
    for (const char* path : titleCandidates) {
        if (FileExists(path)) {
            m_fontTitle = LoadFontEx(path, 64, nullptr, 0);
            if (m_fontTitle.texture.id > 0) {
                SetTextureFilter(m_fontTitle.texture, TEXTURE_FILTER_BILINEAR);
                m_hasCustomFonts = true;
                break;
            }
        }
    }

    // 2. Try loading Body Font
    for (const char* path : bodyCandidates) {
        if (FileExists(path)) {
            m_fontBody = LoadFontEx(path, 48, nullptr, 0);
            if (m_fontBody.texture.id > 0) {
                SetTextureFilter(m_fontBody.texture, TEXTURE_FILTER_BILINEAR);
                m_hasCustomFonts = true;
                break;
            }
        }
    }

    // 3. Try loading Mono Font
    for (const char* path : monoCandidates) {
        if (FileExists(path)) {
            m_fontMono = LoadFontEx(path, 48, nullptr, 0);
            if (m_fontMono.texture.id > 0) {
                SetTextureFilter(m_fontMono.texture, TEXTURE_FILTER_BILINEAR);
                m_hasCustomFonts = true;
                break;
            }
        }
    }

    m_initialized = true;
}

void FontManager::Shutdown() {
    if (!m_initialized) return;

    if (m_fontTitle.texture.id > 0) {
        UnloadFont(m_fontTitle);
        m_fontTitle = {};
    }
    if (m_fontBody.texture.id > 0) {
        UnloadFont(m_fontBody);
        m_fontBody = {};
    }
    if (m_fontMono.texture.id > 0) {
        UnloadFont(m_fontMono);
        m_fontMono = {};
    }

    m_hasCustomFonts = false;
    m_initialized = false;
}

Font FontManager::GetTitleFont() const noexcept {
    if (m_fontTitle.texture.id > 0) return m_fontTitle;
    return GetFontDefault();
}

Font FontManager::GetBodyFont() const noexcept {
    if (m_fontBody.texture.id > 0) return m_fontBody;
    if (m_fontTitle.texture.id > 0) return m_fontTitle;
    return GetFontDefault();
}

Font FontManager::GetMonoFont() const noexcept {
    if (m_fontMono.texture.id > 0) return m_fontMono;
    if (m_fontBody.texture.id > 0) return m_fontBody;
    return GetFontDefault();
}

void FontManager::DrawTitle(const char* text, Vector2 pos, float size, Color color, float spacing) const {
    const Font& font = GetTitleFont();
    DrawTextEx(font, text, pos, size, spacing, color);
}

void FontManager::DrawBody(const char* text, Vector2 pos, float size, Color color, float spacing) const {
    const Font& font = GetBodyFont();
    DrawTextEx(font, text, pos, size, spacing, color);
}

void FontManager::DrawMono(const char* text, Vector2 pos, float size, Color color, float spacing) const {
    const Font& font = GetMonoFont();
    DrawTextEx(font, text, pos, size, spacing, color);
}

void FontManager::DrawGlow(
    const Font& font,
    const char* text,
    Vector2 pos,
    float size,
    Color color,
    Color glowColor,
    float glowRadius,
    float spacing
) const {
    // 4-point halo glow
    Color fadedGlow = Fade(glowColor, 0.35f);
    DrawTextEx(font, text, { pos.x - glowRadius, pos.y }, size, spacing, fadedGlow);
    DrawTextEx(font, text, { pos.x + glowRadius, pos.y }, size, spacing, fadedGlow);
    DrawTextEx(font, text, { pos.x, pos.y - glowRadius }, size, spacing, fadedGlow);
    DrawTextEx(font, text, { pos.x, pos.y + glowRadius }, size, spacing, fadedGlow);

    // Main sharp foreground text
    DrawTextEx(font, text, pos, size, spacing, color);
}

Vector2 FontManager::MeasureTitle(const char* text, float size, float spacing) const {
    const Font& font = GetTitleFont();
    return MeasureTextEx(font, text, size, spacing);
}

Vector2 FontManager::MeasureBody(const char* text, float size, float spacing) const {
    const Font& font = GetBodyFont();
    return MeasureTextEx(font, text, size, spacing);
}

Vector2 FontManager::MeasureMono(const char* text, float size, float spacing) const {
    const Font& font = GetMonoFont();
    return MeasureTextEx(font, text, size, spacing);
}

} // namespace TetroShift
