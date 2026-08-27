#pragma once
#include "VerletNode.hpp"
#include "SpringConstraint.hpp"
#include <vector>
#include <raylib.h>

namespace TetroShift {

class SoftBodyMesh {
public:
    SoftBodyMesh() = default;

    void Initialize(const std::vector<Vector2>& minoWorldPositions);
    void Update(float dt, const std::vector<Vector2>& targetWorldPositions);
    void ApplyImpulse(Vector2 impulse);
    void ApplyRotationTorque(float angularVelocity);
    void TriggerSquish(Vector2 impactVelocity);

    [[nodiscard]] Vector2 GetMinoOffset(size_t minoIndex) const noexcept;
    [[nodiscard]] Vector2 GetMinoRenderPos(size_t minoIndex) const noexcept;
    [[nodiscard]] const std::vector<VerletNode>& GetNodes() const noexcept { return m_nodes; }

    void SetElasticity(float elasticity) noexcept { m_elasticityMultiplier = elasticity; }
    [[nodiscard]] float GetElasticity() const noexcept { return m_elasticityMultiplier; }

    void Render(const std::vector<Vector2>& minoPositions, Color color, float cellSize, bool debugWireframe = false) const;

private:
    std::vector<VerletNode> m_nodes;
    std::vector<SpringConstraint> m_springs;
    float m_elasticityMultiplier = 1.0f;
    float m_springStiffness = 160.0f;
    float m_damping = 12.0f;
};

} // namespace TetroShift
