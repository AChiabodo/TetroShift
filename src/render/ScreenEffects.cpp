#include "ScreenEffects.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <utility>

namespace TetroShift {

// Fallback in-memory GLSL 330 fragment shader
static const char* g_fallbackFragmentShader = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 uResolution;
uniform float uTime;
uniform float uChromaticStrength;
uniform float uBloomIntensity;
uniform float uCrtCurvature;
uniform float uScanlineIntensity;
uniform float uVignetteIntensity;

vec2 CurveCoords(vec2 uv) {
    if (uCrtCurvature <= 0.001) return uv;
    uv = uv * 2.0 - 1.0;
    vec2 offset = abs(uv.yx) / vec2(6.0 / uCrtCurvature, 4.0 / uCrtCurvature);
    uv = uv + uv * offset * offset;
    return uv * 0.5 + 0.5;
}

void main() {
    vec2 uv = CurveCoords(fragTexCoord);
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.02, 0.02, 0.04, 1.0);
        return;
    }

    vec2 chromOffset = vec2(uChromaticStrength, uChromaticStrength * 0.5);
    float r = texture(texture0, uv - chromOffset).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv + chromOffset).b;
    vec4 baseColor = vec4(r, g, b, 1.0);

    if (uBloomIntensity > 0.01) {
        vec4 bloom = vec4(0.0);
        float stepX = 1.8 / uResolution.x;
        float stepY = 1.8 / uResolution.y;

        bloom += texture(texture0, uv + vec2(-stepX * 2.0, -stepY * 2.0)) * 0.08;
        bloom += texture(texture0, uv + vec2( stepX * 2.0, -stepY * 2.0)) * 0.08;
        bloom += texture(texture0, uv + vec2(-stepX * 2.0,  stepY * 2.0)) * 0.08;
        bloom += texture(texture0, uv + vec2( stepX * 2.0,  stepY * 2.0)) * 0.08;

        bloom += texture(texture0, uv + vec2(-stepX, 0.0)) * 0.16;
        bloom += texture(texture0, uv + vec2( stepX, 0.0)) * 0.16;
        bloom += texture(texture0, uv + vec2(0.0, -stepY)) * 0.16;
        bloom += texture(texture0, uv + vec2(0.0,  stepY)) * 0.16;

        vec3 brightColor = max(bloom.rgb - vec3(0.35), vec3(0.0)) * 1.5;
        baseColor.rgb += brightColor * uBloomIntensity;
    }

    if (uScanlineIntensity > 0.01) {
        float scanline = sin((uv.y * uResolution.y * 3.14159265) + (uTime * 4.0)) * 0.5 + 0.5;
        baseColor.rgb -= baseColor.rgb * (1.0 - scanline) * uScanlineIntensity * 0.5;
        float mask = cos(uv.x * uResolution.x * 3.14159265) * 0.03;
        baseColor.rgb += vec3(mask);
    }

    if (uVignetteIntensity > 0.01) {
        vec2 center = uv - vec2(0.5);
        float dist = length(center) * 1.4142;
        float vignette = smoothstep(1.0, 1.0 - uVignetteIntensity, dist);
        baseColor.rgb *= vignette;
    }

    finalColor = baseColor * fragColor;
}
)";

ScreenEffects::ScreenEffects() = default;

ScreenEffects::~ScreenEffects() {
    Shutdown();
}

ScreenEffects::ScreenEffects(ScreenEffects&& other) noexcept {
    *this = std::move(other);
}

ScreenEffects& ScreenEffects::operator=(ScreenEffects&& other) noexcept {
    if (this != &other) {
        Shutdown();

        m_trauma = other.m_trauma;
        m_flashTimer = other.m_flashTimer;
        m_flashMaxDuration = other.m_flashMaxDuration;
        m_flashColor = other.m_flashColor;

        m_scanlinesEnabled = other.m_scanlinesEnabled;
        m_crtCurvatureEnabled = other.m_crtCurvatureEnabled;
        m_bloomEnabled = other.m_bloomEnabled;

        m_shaderTime = other.m_shaderTime;
        m_postShader = other.m_postShader;
        m_shaderLoaded = other.m_shaderLoaded;

        m_locResolution = other.m_locResolution;
        m_locTime = other.m_locTime;
        m_locChromatic = other.m_locChromatic;
        m_locBloom = other.m_locBloom;
        m_locCurvature = other.m_locCurvature;
        m_locScanline = other.m_locScanline;
        m_locVignette = other.m_locVignette;

        other.m_shaderLoaded = false;
        other.m_postShader = {};
    }
    return *this;
}

void ScreenEffects::Initialize(int /*screenWidth*/, int /*screenHeight*/) {
    LoadPostShader();
}

void ScreenEffects::Shutdown() {
    if (m_shaderLoaded) {
        UnloadShader(m_postShader);
        m_shaderLoaded = false;
    }
}

void ScreenEffects::LoadPostShader() {
    if (m_shaderLoaded) return;

    if (FileExists("assets/shaders/crt_bloom.fs")) {
        m_postShader = LoadShader(nullptr, "assets/shaders/crt_bloom.fs");
    } else {
        m_postShader = LoadShaderFromMemory(nullptr, g_fallbackFragmentShader);
    }

    if (m_postShader.id > 0) {
        m_shaderLoaded = true;
        m_locResolution = GetShaderLocation(m_postShader, "uResolution");
        m_locTime = GetShaderLocation(m_postShader, "uTime");
        m_locChromatic = GetShaderLocation(m_postShader, "uChromaticStrength");
        m_locBloom = GetShaderLocation(m_postShader, "uBloomIntensity");
        m_locCurvature = GetShaderLocation(m_postShader, "uCrtCurvature");
        m_locScanline = GetShaderLocation(m_postShader, "uScanlineIntensity");
        m_locVignette = GetShaderLocation(m_postShader, "uVignetteIntensity");
    }
}

void ScreenEffects::Reset() {
    m_trauma = 0.0f;
    m_flashTimer = 0.0f;
    m_flashMaxDuration = 0.3f;
    m_flashColor = WHITE;
}

void ScreenEffects::Update(float dt) {
    m_shaderTime += dt;

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

void ScreenEffects::RenderPostProcessing(const RenderTexture2D& sceneTexture, int screenWidth, int screenHeight) {
    if (!m_shaderLoaded) {
        LoadPostShader();
    }

    if (m_shaderLoaded) {
        float resolution[2] = { static_cast<float>(screenWidth), static_cast<float>(screenHeight) };
        float timeVal = m_shaderTime;
        float chromatic = m_trauma * m_trauma * 0.018f;
        float bloom = m_bloomEnabled ? 0.75f : 0.0f;
        float curvature = m_crtCurvatureEnabled ? 0.045f : 0.0f;
        float scanlines = m_scanlinesEnabled ? 0.35f : 0.0f;
        float vignette = 0.35f;

        SetShaderValue(m_postShader, m_locResolution, resolution, SHADER_UNIFORM_VEC2);
        SetShaderValue(m_postShader, m_locTime, &timeVal, SHADER_UNIFORM_FLOAT);
        SetShaderValue(m_postShader, m_locChromatic, &chromatic, SHADER_UNIFORM_FLOAT);
        SetShaderValue(m_postShader, m_locBloom, &bloom, SHADER_UNIFORM_FLOAT);
        SetShaderValue(m_postShader, m_locCurvature, &curvature, SHADER_UNIFORM_FLOAT);
        SetShaderValue(m_postShader, m_locScanline, &scanlines, SHADER_UNIFORM_FLOAT);
        SetShaderValue(m_postShader, m_locVignette, &vignette, SHADER_UNIFORM_FLOAT);

        BeginShaderMode(m_postShader);
        // Note: OpenGL render textures are vertically flipped, so height must be negative
        Rectangle srcRec = { 0.0f, 0.0f, static_cast<float>(sceneTexture.texture.width), -static_cast<float>(sceneTexture.texture.height) };
        Rectangle dstRec = { 0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight) };
        DrawTexturePro(sceneTexture.texture, srcRec, dstRec, { 0.0f, 0.0f }, 0.0f, WHITE);
        EndShaderMode();
    } else {
        // Simple texture draw if shader is not supported
        Rectangle srcRec = { 0.0f, 0.0f, static_cast<float>(sceneTexture.texture.width), -static_cast<float>(sceneTexture.texture.height) };
        Rectangle dstRec = { 0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight) };
        DrawTexturePro(sceneTexture.texture, srcRec, dstRec, { 0.0f, 0.0f }, 0.0f, WHITE);
        RenderPostProcessing(screenWidth, screenHeight);
    }

    // Flash Overlay
    if (m_flashTimer > 0.0f) {
        float alpha = m_flashTimer / m_flashMaxDuration;
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(m_flashColor, alpha * 0.45f));
    }
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
