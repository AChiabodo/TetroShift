#include "SoftBodyMesh.hpp"
#include "core/Constants.hpp"
#include <raymath.h>
#include <algorithm>

namespace TetroShift {

void SoftBodyMesh::Initialize(const std::vector<Vector2>& minoWorldPositions) {
    const size_t count = minoWorldPositions.size();
    m_nodes.resize(count);
    m_springs.clear();

    for (size_t i = 0; i < count; ++i) {
        m_nodes[i].position = minoWorldPositions[i];
        m_nodes[i].previousPosition = minoWorldPositions[i];
        m_nodes[i].acceleration = { 0.0f, 0.0f };
        m_nodes[i].mass = 1.0f;
        m_nodes[i].isPinned = false;
    }

    // Connect pairs with springs
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j < count; ++j) {
            float dist = Vector2Distance(minoWorldPositions[i], minoWorldPositions[j]);
            m_springs.push_back(SpringConstraint{
                static_cast<int>(i),
                static_cast<int>(j),
                dist,
                0.6f * m_elasticityMultiplier
            });
        }
    }
}

void SoftBodyMesh::Update(float dt, const std::vector<Vector2>& targetWorldPositions) {
    if (m_nodes.size() != targetWorldPositions.size()) {
        Initialize(targetWorldPositions);
        return;
    }

    const float kAnchor = m_springStiffness / std::max(0.2f, m_elasticityMultiplier);
    const float dAnchor = m_damping;

    // Apply restorative force towards logical grid positions
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        VerletNode& node = m_nodes[i];
        const Vector2& target = targetWorldPositions[i];

        Vector2 displacement = { node.position.x - target.x, node.position.y - target.y };
        Vector2 velocity = {
            (node.position.x - node.previousPosition.x) / std::max(0.0001f, dt),
            (node.position.y - node.previousPosition.y) / std::max(0.0001f, dt)
        };

        Vector2 springForce = {
            -kAnchor * displacement.x - dAnchor * velocity.x,
            -kAnchor * displacement.y - dAnchor * velocity.y
        };

        node.ApplyForce(springForce);
        node.Update(dt, 0.94f);
    }

    // Solve internal distance constraints multiple iterations for stability
    const int iterations = 3;
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& spring : m_springs) {
            spring.Solve(m_nodes.data(), m_nodes.size());
        }
    }
}

void SoftBodyMesh::ApplyImpulse(Vector2 impulse) {
    for (auto& node : m_nodes) {
        node.previousPosition.x -= impulse.x * 0.05f * m_elasticityMultiplier;
        node.previousPosition.y -= impulse.y * 0.05f * m_elasticityMultiplier;
    }
}

void SoftBodyMesh::ApplyRotationTorque(float angularVelocity) {
    if (m_nodes.empty()) return;

    // Compute centroid
    Vector2 center = { 0.0f, 0.0f };
    for (const auto& node : m_nodes) {
        center.x += node.position.x;
        center.y += node.position.y;
    }
    center.x /= static_cast<float>(m_nodes.size());
    center.y /= static_cast<float>(m_nodes.size());

    // Tangential impulse
    for (auto& node : m_nodes) {
        Vector2 r = { node.position.x - center.x, node.position.y - center.y };
        Vector2 tangent = { -r.y * angularVelocity * 0.02f, r.x * angularVelocity * 0.02f };
        node.previousPosition.x -= tangent.x * m_elasticityMultiplier;
        node.previousPosition.y -= tangent.y * m_elasticityMultiplier;
    }
}

void SoftBodyMesh::TriggerSquish(Vector2 impactVelocity) {
    for (auto& node : m_nodes) {
        // Vertical compression and horizontal expansion
        node.previousPosition.y -= impactVelocity.y * 0.06f * m_elasticityMultiplier;
        node.previousPosition.x += (impactVelocity.y * 0.03f) * ((node.position.x > 0) ? 1.0f : -1.0f) * m_elasticityMultiplier;
    }
}

Vector2 SoftBodyMesh::GetMinoOffset(size_t minoIndex) const noexcept {
    if (minoIndex < m_nodes.size()) {
        return { m_nodes[minoIndex].position.x, m_nodes[minoIndex].position.y };
    }
    return { 0.0f, 0.0f };
}

Vector2 SoftBodyMesh::GetMinoRenderPos(size_t minoIndex) const noexcept {
    if (minoIndex < m_nodes.size()) {
        return m_nodes[minoIndex].position;
    }
    return { 0.0f, 0.0f };
}

void SoftBodyMesh::Render(const std::vector<Vector2>& minoPositions, Color color, float cellSize, bool debugWireframe) const {
    const size_t count = minoPositions.size();

    // Render deformed soft-body blocks
    for (size_t i = 0; i < count; ++i) {
        Vector2 renderPos = (i < m_nodes.size()) ? m_nodes[i].position : minoPositions[i];
        const Rectangle rect = { renderPos.x + 1.0f, renderPos.y + 1.0f, cellSize - 2.0f, cellSize - 2.0f };

        // Rounded body
        DrawRectangleRounded(rect, 0.22f, 4, color);

        // Highlight sheen
        DrawRectangleRec({ rect.x + 2.0f, rect.y + 2.0f, rect.width - 4.0f, 3.0f }, Fade(WHITE, 0.45f));
        // Shadow base
        DrawRectangleRec({ rect.x + 2.0f, rect.y + rect.height - 4.0f, rect.width - 4.0f, 2.0f }, Fade(BLACK, 0.35f));
    }

    // Render spring constraints in debug mode
    if (debugWireframe) {
        for (const auto& spring : m_springs) {
            if (static_cast<size_t>(spring.nodeA) < m_nodes.size() && static_cast<size_t>(spring.nodeB) < m_nodes.size()) {
                Vector2 p1 = m_nodes[spring.nodeA].position;
                Vector2 p2 = m_nodes[spring.nodeB].position;
                p1.x += cellSize * 0.5f;
                p1.y += cellSize * 0.5f;
                p2.x += cellSize * 0.5f;
                p2.y += cellSize * 0.5f;
                DrawLineV(p1, p2, Fade(MAGENTA, 0.7f));
            }
        }
        for (const auto& node : m_nodes) {
            DrawCircle(static_cast<int>(node.position.x + cellSize * 0.5f), static_cast<int>(node.position.y + cellSize * 0.5f), 3.0f, YELLOW);
        }
    }
}

} // namespace TetroShift
