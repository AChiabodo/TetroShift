#pragma once
#include "IGameState.hpp"
#include "MenuTypes.hpp"
#include "render/MenuRenderer.hpp"
#include <raylib.h>
#include <vector>
#include <string>

namespace TetroShift {

class TitleState : public IGameState {
public:
    TitleState() = default;
    ~TitleState() override = default;

    void OnEnter(GameApp& app) override;
    void OnExit(GameApp& app) override;
    void Update(GameApp& app, float dt) override;
    void Render(GameApp& app) override;
    void HandleInput(GameApp& app) override;

private:
    // View state & Transitions
    void SetView(MenuView view, GameApp& app);
    void GoBack(GameApp& app);

    // Sub-view Renders
    void RenderMainHub(GameApp& app);
    void RenderPlaySelect(GameApp& app);
    void RenderProfileSaves(GameApp& app);
    void RenderHighScores(GameApp& app);
    void RenderShop(GameApp& app);
    void RenderCustomization(GameApp& app);
    void RenderSettings(GameApp& app);
    void RenderQuitConfirm(GameApp& app);

    // Sub-view Inputs
    void HandleInputMainHub(GameApp& app);
    void HandleInputPlaySelect(GameApp& app);
    void HandleInputProfileSaves(GameApp& app);
    void HandleInputHighScores(GameApp& app);
    void HandleInputShop(GameApp& app);
    void HandleInputCustomization(GameApp& app);
    void HandleInputSettings(GameApp& app);
    void HandleInputQuitConfirm(GameApp& app);

    // Data initialization helpers
    void InitializeMockData();

    // State Variables
    MenuView m_currentView = MenuView::Main;
    int m_selectedOption = 0;
    int m_activeTab = 0;
    float m_animTimer = 0.0f;
    float m_transitionAlpha = 1.0f;

    // Subsystem components & datasets
    MenuRenderer m_menuRenderer;
    GameSettings m_settings;
    PlayerProfileData m_profile;
    std::vector<SaveSlotData> m_saveSlots;
    std::vector<HighScoreEntry> m_highScores;
    std::vector<ShopItemData> m_shopItems;
    std::vector<CustomThemeData> m_customThemes;
    int m_selectedThemeIndex = 0;
    int m_selectedJukeboxIndex = 0;
};

} // namespace TetroShift
