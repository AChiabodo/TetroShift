#pragma once
#include <raymath.h>
#include <cmath>

namespace TetroShift {

struct SpringConstraint {
    int nodeA = 0;
    int nodeB = 0;
    float restLength = 0.0f;
    float stiffness = 0.5f; // 0.0 (weak) to 1.0 (rigid)

    void Solve(struct VerletNode* nodes, size_t nodeCount) const noexcept {
        if (nodeA < 0 || nodeB < 0 || static_cast<size_t>(nodeA) >= nodeCount || static_cast<size_t>(nodeB) >= nodeCount) {
            return;
        }

        VerletNode& a = nodes[nodeA];
        VerletNode& b = nodes[nodeB];

        Vector2 delta = { b.position.x - a.position.x, b.position.y - a.position.y };
        float dist = Vector2Length(delta);
        if (dist < 0.0001f) return;

        float diff = (dist - restLength) / dist;
        Vector2 correction = { delta.x * 0.5f * diff * stiffness, delta.y * 0.5f * diff * stiffness };

        if (!a.isPinned && !b.isPinned) {
            a.position.x += correction.x;
            a.position.y += correction.y;
            b.position.x -= correction.x;
            b.position.y -= correction.y;
        } else if (!a.isPinned && b.isPinned) {
            a.position.x += correction.x * 2.0f;
            a.position.y += correction.y * 2.0f;
        } else if (a.isPinned && !b.isPinned) {
            b.position.x -= correction.x * 2.0f;
            b.position.y -= correction.y * 2.0f;
        }
    }
};

} // namespace TetroShift
