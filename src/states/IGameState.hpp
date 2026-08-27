#pragma once

namespace TetroShift {

class GameApp;

class IGameState {
public:
    virtual ~IGameState() = default;

    virtual void OnEnter(GameApp& app) = 0;
    virtual void OnExit(GameApp& app) = 0;
    virtual void Update(GameApp& app, float dt) = 0;
    virtual void Render(GameApp& app) = 0;
    virtual void HandleInput(GameApp& app) = 0;
};

} // namespace TetroShift
