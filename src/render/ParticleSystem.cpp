#include "ParticleSystem.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>

namespace TetroShift {

void ParticleSystem::Reset() {
    m_particles.clear();
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
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = center;

        float angle = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 6.283185f;
        float speed = 100.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 320.0f;
        p.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };

        p.color = (i % 2 == 0) ? color : ORANGE;
        p.size = 4.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 6.0f;
        p.maxLife = 0.6f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.6f;
        p.life = p.maxLife;
        p.hasGravity = false;

        m_particles.push_back(p);
    }
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
