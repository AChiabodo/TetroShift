#pragma once
#include <raylib.h>

namespace TetroShift {

struct VerletNode {
    Vector2 position = { 0.0f, 0.0f };
    Vector2 previousPosition = { 0.0f, 0.0f };
    Vector2 restOffset = { 0.0f, 0.0f }; // Local offset from piece root
    Vector2 acceleration = { 0.0f, 0.0f };
    float mass = 1.0f;
    bool isPinned = false;

    void ApplyForce(Vector2 force) noexcept {
        if (!isPinned && mass > 0.0f) {
            acceleration.x += force.x / mass;
            acceleration.y += force.y / mass;
        }
    }

    void Update(float dt, float damping = 0.96f) noexcept {
        if (isPinned) {
            previousPosition = position;
            acceleration = { 0.0f, 0.0f };
            return;
        }

        Vector2 velocity = {
            (position.x - previousPosition.x) * damping,
            (position.y - previousPosition.y) * damping
        };

        previousPosition = position;
        position.x += velocity.x + acceleration.x * dt * dt;
        position.y += velocity.y + acceleration.y * dt * dt;
        acceleration = { 0.0f, 0.0f };
    }
};

} // namespace TetroShift
