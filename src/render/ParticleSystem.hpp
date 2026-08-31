#pragma once
#include <raylib.h>
#include <vector>
#include <string>

namespace TetroShift {

struct Particle {
    Vector2 position = { 0.0f, 0.0f };
    Vector2 velocity = { 0.0f, 0.0f };
    Color color = WHITE;
    float size = 4.0f;
    float life = 1.0f;
    float maxLife = 1.0f;
    bool hasGravity = true;
};

struct FloatingText {
    std::string text;
    Vector2 position = { 0.0f, 0.0f };
    Vector2 velocity = { 0.0f, -40.0f };
    Color color = WHITE;
    float scale = 1.0f;
    float life = 1.2f;
    float maxLife = 1.2f;
};

struct ShockwaveRing {
    Vector2 center = { 0.0f, 0.0f };
    float currentRadius = 0.0f;
    float maxRadius = 120.0f;
    float life = 0.5f;
    float maxLife = 0.5f;
    Color color = RED;
};

class ParticleSystem {
public:
    ParticleSystem() = default;

    void Reset();
    void Update(float dt);
    void Render() const;

    void EmitLineClear(float rowWorldY, float startX, float endX, Color color, int count = 40);
    void EmitBombBlast(Vector2 center, Color color, int count = 60);
    void EmitShockwaveRing(Vector2 center, Color color, float maxRadius = 120.0f);
    void EmitHardDropDust(Vector2 landingPos, Color color, int count = 15);
    void EmitSandDust(Vector2 pos, int count = 12);
    void EmitMidasSparkles(Vector2 pos, int count = 8);

    void AddPopup(const std::string& text, Vector2 position, Color color, float scale = 1.0f);

private:
    std::vector<Particle> m_particles;
    std::vector<ShockwaveRing> m_shockwaves;
    std::vector<FloatingText> m_popups;
};

} // namespace TetroShift
