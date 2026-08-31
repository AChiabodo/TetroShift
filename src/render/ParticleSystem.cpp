#include "ParticleSystem.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>

namespace TetroShift {

void ParticleSystem::Reset() {
    m_particles.clear();
    m_shockwaves.clear();
    m_popups.clear();
}

void ParticleSystem::Update(float dt) {
    // Update Particles
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
        } else {
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            if (it->hasGravity) {
                it->velocity.y += 400.0f * dt; // Gravity
            }
            it->velocity.x *= 0.98f; // Friction
            ++it;
        }
    }

    // Update Shockwave Rings
    for (auto it = m_shockwaves.begin(); it != m_shockwaves.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = m_shockwaves.erase(it);
        } else {
            float progress = 1.0f - (it->life / it->maxLife);
            it->currentRadius = it->maxRadius * progress;
            ++it;
        }
    }

    // Update Floating Popups
    for (auto it = m_popups.begin(); it != m_popups.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = m_popups.erase(it);
        } else {
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            it->velocity.y *= 0.95f;
            ++it;
        }
    }
}

void ParticleSystem::EmitLineClear(float rowWorldY, float startX, float endX, Color color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        float rx = startX + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * (endX - startX);
        p.position = { rx, rowWorldY + 16.0f };

        float angle = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 6.283185f;
        float speed = 60.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 220.0f;
        p.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };

        p.color = color;
        p.size = 3.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 5.0f;
        p.maxLife = 0.5f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.5f;
        p.life = p.maxLife;
        p.hasGravity = true;

        m_particles.push_back(p);
    }
}

void ParticleSystem::EmitBombBlast(Vector2 center, Color color, int count) {
    EmitShockwaveRing(center, color, 140.0f);

    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = center;

        float angle = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 6.283185f;
        float speed = 100.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 340.0f;
        p.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };

        p.color = (i % 3 == 0) ? YELLOW : ((i % 2 == 0) ? color : ORANGE);
        p.size = 4.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 7.0f;
        p.maxLife = 0.6f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.6f;
        p.life = p.maxLife;
        p.hasGravity = true;

        m_particles.push_back(p);
    }
}

void ParticleSystem::EmitShockwaveRing(Vector2 center, Color color, float maxRadius) {
    ShockwaveRing ring;
    ring.center = center;
    ring.currentRadius = 0.0f;
    ring.maxRadius = maxRadius;
    ring.life = 0.45f;
    ring.maxLife = 0.45f;
    ring.color = color;
    m_shockwaves.push_back(ring);
}

void ParticleSystem::EmitHardDropDust(Vector2 landingPos, Color color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = landingPos;

        float dir = (i % 2 == 0) ? -1.0f : 1.0f;
        float vx = dir * (40.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 120.0f);
        float vy = -(30.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 90.0f);
        p.velocity = { vx, vy };

        p.color = Fade(color, 0.7f);
        p.size = 2.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 4.0f;
        p.maxLife = 0.3f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.3f;
        p.life = p.maxLife;
        p.hasGravity = true;

        m_particles.push_back(p);
    }
}

void ParticleSystem::EmitSandDust(Vector2 pos, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = {
            pos.x + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 16.0f,
            pos.y + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 16.0f
        };
        p.velocity = {
            (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 40.0f,
            -(10.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 30.0f)
        };
        p.color = Color{ 235, 185, 90, 220 };
        p.size = 2.0f;
        p.maxLife = 0.35f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.25f;
        p.life = p.maxLife;
        p.hasGravity = true;
        m_particles.push_back(p);
    }
}

void ParticleSystem::EmitMidasSparkles(Vector2 pos, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = {
            pos.x + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 24.0f,
            pos.y + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 24.0f
        };
        p.velocity = {
            (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 30.0f,
            -(20.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 40.0f)
        };
        p.color = GOLD;
        p.size = 3.0f;
        p.maxLife = 0.4f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.3f;
        p.life = p.maxLife;
        p.hasGravity = false;
        m_particles.push_back(p);
    }
}

void ParticleSystem::AddPopup(const std::string& text, Vector2 position, Color color, float scale) {
    FloatingText f;
    f.text = text;
    f.position = position;
    f.color = color;
    f.scale = scale;
    f.life = 1.2f;
    f.maxLife = 1.2f;
    m_popups.push_back(f);
}

void ParticleSystem::Render() const {
    // Render Shockwave Rings
    for (const auto& ring : m_shockwaves) {
        float alpha = ring.life / ring.maxLife;
        Color drawCol = Fade(ring.color, alpha * 0.8f);
        DrawCircleLines(static_cast<int>(ring.center.x), static_cast<int>(ring.center.y), ring.currentRadius, drawCol);
        if (ring.currentRadius > 3.0f) {
            DrawCircleLines(static_cast<int>(ring.center.x), static_cast<int>(ring.center.y), ring.currentRadius - 2.0f, Fade(drawCol, 0.5f));
        }
    }

    // Render Particles
    for (const auto& p : m_particles) {
        float alpha = p.life / p.maxLife;
        Color drawCol = Fade(p.color, alpha);
        DrawRectangleV(p.position, { p.size, p.size }, drawCol);
    }

    // Render Floating Popups
    for (const auto& popup : m_popups) {
        float alpha = popup.life / popup.maxLife;
        int fontSize = static_cast<int>(20.0f * popup.scale);
        Color drawCol = Fade(popup.color, alpha);

        int textWidth = MeasureText(popup.text.c_str(), fontSize);
        DrawText(popup.text.c_str(),
                 static_cast<int>(popup.position.x - textWidth * 0.5f),
                 static_cast<int>(popup.position.y),
                 fontSize,
                 drawCol);
    }
}

} // namespace TetroShift
