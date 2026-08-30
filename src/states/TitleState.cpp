#include "TitleState.hpp"
#include "PlayState.hpp"
#include "core/GameApp.hpp"
#include "core/Constants.hpp"
#include <cmath>
#include <algorithm>

namespace TetroShift {

void TitleState::OnEnter(GameApp& app) {
    m_animTimer = 0.0f;
    m_currentView = MenuView::Main;
    m_selectedOption = 0;
    m_activeTab = 0;
    m_selectedThemeIndex = 0;

    InitializeMockData();
    app.GetSoundSynth().SetMasterVolume(m_settings.masterVolume);
}

void TitleState::OnExit(GameApp& /*app*/) {}

void TitleState::InitializeMockData() {
    // 1. Profile Data
    m_profile.pilotName = "CYBER_PILOT_01";
    m_profile.pilotCallsign = "MORPHO-PRIME";
    m_profile.rankTitle = "MASTER ARCHITECT III";
    m_profile.level = 14;
    m_profile.currentExp = 3850;
    m_profile.maxExp = 5000;
    m_profile.totalRuns = 42;
    m_profile.totalVictories = 18;
    m_profile.totalLinesCleared = 2450;
    m_profile.highestScore = 184200;
    m_profile.highestFloor = 12;
    m_profile.energyCredits = 1450;

    // 2. Save Slots
    m_saveSlots.clear();
    SaveSlotData slot1;
    slot1.slotId = 1;
    slot1.state = SaveSlotState::ActiveRun;
    slot1.runMode = "ROGUELIKE RUN // SECTOR 04";
    slot1.currentFloor = 4;
    slot1.currentScore = 48200;
    slot1.currentLines = 36;
    slot1.energyCoins = 180;
    slot1.timestamp = "2026-08-30 22:15";
    slot1.relicCards = { "JELLY BODY", "MIDAS TOUCH", "FISSION" };
    slot1.accentColor = Colors::PieceI;
    m_saveSlots.push_back(slot1);

    SaveSlotData slot2;
    slot2.slotId = 2;
    slot2.state = SaveSlotState::Empty;
    slot2.runMode = "EMPTY MEMORY MATRIX";
    slot2.timestamp = "--/--/----";
    slot2.accentColor = Colors::TextDim;
    m_saveSlots.push_back(slot2);

    SaveSlotData slot3;
    slot3.slotId = 3;
    slot3.state = SaveSlotState::CompletedRun;
    slot3.runMode = "ENDLESS MATRIX // ARCHIVE";
    slot3.currentFloor = 10;
    slot3.currentScore = 142500;
    slot3.currentLines = 112;
    slot3.energyCoins = 520;
    slot3.timestamp = "2026-08-28 19:40";
    slot3.relicCards = { "HEAVY IRON", "MAGNET LEFT" };
    slot3.accentColor = Colors::TextGold;
    m_saveSlots.push_back(slot3);

    // 3. High Scores
    m_highScores = {
        { 1, "CYBER_PILOT_01", 184200, 12, 148, "2026-08-29", "GRAND MASTER", GOLD },
        { 2, "NEON_VIPER", 152800, 10, 120, "2026-08-28", "WARP MASTER", LIGHTGRAY },
        { 3, "SYNTH_ZERO", 128400, 8, 96, "2026-08-26", "ELITE RUNNER", Colors::PieceL },
        { 4, "MATRIX_GHOST", 98200, 7, 82, "2026-08-25", "VETERAN", Colors::PieceT },
        { 5, "QUANTUM_KID", 64500, 5, 54, "2026-08-22", "INITIATE", Colors::PieceS }
    };

    // 4. Shop Items (Black Market)
    m_shopItems = {
        { "SHOP_JELLY_V2", "SUPER JELLY COATING", ShopCategory::Relic, "RELIC // PASSIVE", "+30% spring bounce and self-stabilizing lock", 350, false, false, Colors::PieceJelly },
        { "SHOP_START_COINS", "MIDAS CATALYST", ShopCategory::Booster, "BOOSTER // ACTIVE", "Start every new run with +100 Energy Credits", 500, false, false, Colors::PieceGold },
        { "SHOP_SKIN_CYBER", "CYBERPUNK NEON SKIN", ShopCategory::Skin, "COSMETIC // THEME", "High-glow chromatic aberration block theme", 400, true, true, Colors::PieceI },
        { "SHOP_SKIN_TITAN", "TITAN METALLIC SKIN", ShopCategory::Skin, "COSMETIC // THEME", "Heavy brushed titanium aesthetics with sparks", 650, false, false, Colors::PieceIron },
        { "SHOP_REROLL_PACK", "QUANTUM REROLL MATRIX", ShopCategory::Booster, "BOOSTER // PERK", "+2 Draft card reroll tokens per run", 450, false, false, Colors::PieceT },
        { "SHOP_CRT_PULSE", "SYNTHWAVE GRID THEME", ShopCategory::MatrixTheme, "COSMETIC // MATRIX", "Undulating retro-futuristic horizon matrix grid", 300, true, false, Colors::PieceZ }
    };

    // 5. Customization Themes
    m_customThemes = {
        { "THEME_NEON", "CYBER NEON (DEFAULT)", "High-contrast electric neon with cyan and violet accents", Colors::PieceI, Colors::PieceT, TetrominoType::T, true, true },
        { "THEME_TITAN", "TITAN INDUSTRIAL", "Heavy metallic frames with intense industrial orange glow", Colors::PieceIron, Colors::PieceL, TetrominoType::I, false, false },
        { "THEME_JELLY", "GELATIN LUMINESCENCE", "Soft aquamarine translucent blocks with harmonic wobble", Colors::PieceJelly, Colors::PieceS, TetrominoType::S, true, false },
        { "THEME_RETRO", "ARCADE SYNTH 1984", "Classic vibrant primaries with retro crt phosphor edge", Colors::PieceO, Colors::PieceBomb, TetrominoType::L, false, false }
    };
}

void TitleState::SetView(MenuView view, GameApp& app) {
    m_currentView = view;
    m_selectedOption = 0;
    app.GetSoundSynth().PlayCardSelect();
}

void TitleState::GoBack(GameApp& app) {
    if (m_currentView == MenuView::Main) {
        SetView(MenuView::QuitConfirm, app);
    } else {
        m_currentView = MenuView::Main;
        m_selectedOption = 0;
        app.GetSoundSynth().PlayMenuBack();
    }
}

void TitleState::Update(GameApp& /*app*/, float dt) {
    m_animTimer += dt;
}

void TitleState::HandleInput(GameApp& app) {
    switch (m_currentView) {
        case MenuView::Main:          HandleInputMainHub(app); break;
        case MenuView::PlaySelect:    HandleInputPlaySelect(app); break;
        case MenuView::ProfileSaves:  HandleInputProfileSaves(app); break;
        case MenuView::HighScores:    HandleInputHighScores(app); break;
        case MenuView::Shop:          HandleInputShop(app); break;
        case MenuView::Customization: HandleInputCustomization(app); break;
        case MenuView::Settings:      HandleInputSettings(app); break;
        case MenuView::QuitConfirm:   HandleInputQuitConfirm(app); break;
    }
}

void TitleState::HandleInputMainHub(GameApp& app) {
    const int numOptions = 7;

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + numOptions) % numOptions;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % numOptions;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        SetView(MenuView::QuitConfirm, app);
        return;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        switch (m_selectedOption) {
            case 0: SetView(MenuView::PlaySelect, app); break;
            case 1: SetView(MenuView::ProfileSaves, app); break;
            case 2: SetView(MenuView::HighScores, app); break;
            case 3: SetView(MenuView::Shop, app); break;
            case 4: SetView(MenuView::Customization, app); break;
            case 5: SetView(MenuView::Settings, app); break;
            case 6: SetView(MenuView::QuitConfirm, app); break;
        }
    }
}

void TitleState::HandleInputPlaySelect(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE)) { GoBack(app); return; }

    const int numOptions = 2;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + numOptions) % numOptions;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % numOptions;
        app.GetSoundSynth().PlayMenuHover();
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        app.GetSoundSynth().PlayCardSelect();
        // Start run in PlayState
        app.GetStateManager().SetState(app, std::make_unique<PlayState>());
    }
}

void TitleState::HandleInputProfileSaves(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE)) { GoBack(app); return; }

    const int numSlots = static_cast<int>(m_saveSlots.size());
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + numSlots) % numSlots;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % numSlots;
        app.GetSoundSynth().PlayMenuHover();
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        app.GetSoundSynth().PlayCardSelect();
        // Launch or resume session
        app.GetStateManager().SetState(app, std::make_unique<PlayState>());
    }
}

void TitleState::HandleInputHighScores(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE)) { GoBack(app); return; }

    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
        m_activeTab = (m_activeTab + 1) % 3;
        app.GetSoundSynth().PlayMenuToggle();
    }
}

void TitleState::HandleInputShop(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE)) { GoBack(app); return; }

    const int numItems = static_cast<int>(m_shopItems.size());
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + numItems) % numItems;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % numItems;
        app.GetSoundSynth().PlayMenuHover();
    }

    // Buy / Equip
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (m_selectedOption >= 0 && m_selectedOption < numItems) {
            auto& item = m_shopItems[m_selectedOption];
            if (!item.isUnlocked) {
                if (m_profile.energyCredits >= item.cost) {
                    m_profile.energyCredits -= item.cost;
                    item.isUnlocked = true;
                    item.isEquipped = true;
                    app.GetSoundSynth().PlayLevelUp();
                } else {
                    app.GetSoundSynth().PlayMenuBack();
                }
            } else {
                item.isEquipped = !item.isEquipped;
                app.GetSoundSynth().PlayMenuToggle();
            }
        }
    }
}

void TitleState::HandleInputCustomization(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE)) { GoBack(app); return; }

    const int numThemes = static_cast<int>(m_customThemes.size());
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + numThemes) % numThemes;
        m_selectedThemeIndex = m_selectedOption;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % numThemes;
        m_selectedThemeIndex = m_selectedOption;
        app.GetSoundSynth().PlayMenuHover();
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        for (size_t i = 0; i < m_customThemes.size(); ++i) {
            m_customThemes[i].isEquipped = (static_cast<int>(i) == m_selectedOption);
        }
        app.GetSoundSynth().PlayCardSelect();
    }
}

void TitleState::HandleInputSettings(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE)) { GoBack(app); return; }

    const int numSettings = 5;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + numSettings) % numSettings;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % numSettings;
        app.GetSoundSynth().PlayMenuHover();
    }

    // Left / Right adjustments
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        if (m_selectedOption == 0) { // Volume
            m_settings.masterVolume = std::max(0.0f, m_settings.masterVolume - 0.1f);
            app.GetSoundSynth().SetMasterVolume(m_settings.masterVolume);
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 1) { // Screen Shake
            m_settings.screenShakeLevel = (m_settings.screenShakeLevel - 1 + 4) % 4;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 2) { // CRT Scanlines
            m_settings.crtScanlines = !m_settings.crtScanlines;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 3) { // Soft-Body Wobble
            m_settings.softBodyWobble = (m_settings.softBodyWobble - 1 + 3) % 3;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 4) { // Fast DAS
            m_settings.fastDAS = !m_settings.fastDAS;
            app.GetSoundSynth().PlayMenuToggle();
        }
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (m_selectedOption == 0) { // Volume
            m_settings.masterVolume = std::min(1.0f, m_settings.masterVolume + 0.1f);
            app.GetSoundSynth().SetMasterVolume(m_settings.masterVolume);
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 1) { // Screen Shake
            m_settings.screenShakeLevel = (m_settings.screenShakeLevel + 1) % 4;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 2) { // CRT Scanlines
            m_settings.crtScanlines = !m_settings.crtScanlines;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 3) { // Soft-Body Wobble
            m_settings.softBodyWobble = (m_settings.softBodyWobble + 1) % 3;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 4) { // Fast DAS
            m_settings.fastDAS = !m_settings.fastDAS;
            app.GetSoundSynth().PlayMenuToggle();
        }
    }
}

void TitleState::HandleInputQuitConfirm(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_N)) {
        SetView(MenuView::Main, app);
        return;
    }

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D)) {
        m_selectedOption = (m_selectedOption == 0) ? 1 : 0;
        app.GetSoundSynth().PlayMenuHover();
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_Y)) {
        if (m_selectedOption == 1) {
            CloseWindow();
        } else {
            SetView(MenuView::Main, app);
        }
    }
}

void TitleState::Render(GameApp& app) {
    // 1. Animated Atmospheric Backdrop
    m_menuRenderer.DrawAnimatedBackground(m_animTimer);

    // 2. Render Active Sub-View
    switch (m_currentView) {
        case MenuView::Main:          RenderMainHub(app); break;
        case MenuView::PlaySelect:    RenderPlaySelect(app); break;
        case MenuView::ProfileSaves:  RenderProfileSaves(app); break;
        case MenuView::HighScores:    RenderHighScores(app); break;
        case MenuView::Shop:          RenderShop(app); break;
        case MenuView::Customization: RenderCustomization(app); break;
        case MenuView::Settings:      RenderSettings(app); break;
        case MenuView::QuitConfirm:
            RenderMainHub(app);
            RenderQuitConfirm(app);
            break;
    }
}

void TitleState::RenderMainHub(GameApp& app) {
    Vector2 mousePos = GetMousePosition();

    // 1. Top Player Bar (Right aligned with ample space)
    Rectangle playerBarRect = { WINDOW_WIDTH - 460.0f, 18.0f, 412.0f, 62.0f };
    m_menuRenderer.DrawPlayerStatusBar(m_profile, playerBarRect);

    // 2. Glowing Hero Logo & Subtitle (Left column with clean sizing)
    const char* logo = "TETROSHIFT";
    float glowPulse = (std::sin(m_animTimer * 2.8f) + 1.0f) * 0.5f;
    Color glowCol = ColorAlphaBlend(Colors::PieceI, WHITE, Fade(WHITE, glowPulse * 0.35f));

    DrawText(logo, 48, 40, 48, glowCol);

    const char* subtitle = "MORPHOTETRIS // ROGUELIKE SOFT-BODY ENGINE";
    DrawText(subtitle, 50, 94, 12, Colors::TextDim);

    const char* versionTag = "v1.2.0-EA  *  RAYLIB 5.0  *  C++20 ARCHITECTURE";
    DrawText(versionTag, 50, 114, 10, Colors::TextAccent);

    // 3. Menu Navigation List (Left Column)
    struct MenuOption {
        const char* label;
        const char* badge;
        Color accent;
    };

    const MenuOption options[] = {
        { "START MISSION", "PLAY", Colors::TextGreen },
        { "PILOT DOSSIER & SAVES", "SLOTS 1-3", Colors::PieceI },
        { "HALL OF FAME", "RECORDS", Colors::PieceGold },
        { "DATA MARKET SHOP", "UPGRADES", Colors::PieceT },
        { "HANGAR CUSTOMIZATION", "THEMES", Colors::PieceS },
        { "SYSTEM SETTINGS", "AUDIO/GFX", Colors::TextDim },
        { "DISENGAGE & EXIT", "QUIT", Colors::PieceBomb }
    };

    float startY = 144.0f;
    float btnH = 48.0f;
    float btnSpacing = 9.0f;

    for (int i = 0; i < 7; ++i) {
        Rectangle btnRect = { 48.0f, startY + static_cast<float>(i) * (btnH + btnSpacing), 372.0f, btnH };
        bool isHovered = CheckCollisionPointRec(mousePos, btnRect);
        bool isSelected = (m_selectedOption == i);

        if (isHovered && !isSelected) {
            m_selectedOption = i;
            app.GetSoundSynth().PlayMenuHover();
        }

        if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            switch (i) {
                case 0: SetView(MenuView::PlaySelect, app); break;
                case 1: SetView(MenuView::ProfileSaves, app); break;
                case 2: SetView(MenuView::HighScores, app); break;
                case 3: SetView(MenuView::Shop, app); break;
                case 4: SetView(MenuView::Customization, app); break;
                case 5: SetView(MenuView::Settings, app); break;
                case 6: SetView(MenuView::QuitConfirm, app); break;
            }
        }

        m_menuRenderer.DrawNeonButton(
            btnRect,
            options[i].label,
            options[i].badge,
            isSelected,
            isHovered,
            options[i].accent
        );
    }

    // 4. Right Side Feature Hero Box (Separated cleanly without overlapping logo)
    Rectangle heroCard = { 440.0f, 96.0f, WINDOW_WIDTH - 488.0f, 595.0f };
    DrawRectangleRounded(heroCard, 0.04f, 6, Fade(Colors::BgPanel, 0.85f));
    DrawRectangleLinesEx(heroCard, 1.5f, Colors::BgPanelBorder);

    // Hero Header inside card
    DrawText("FEATURE MATRIX // HIGHLIGHTS", static_cast<int>(heroCard.x + 28.0f), static_cast<int>(heroCard.y + 24.0f), 15, Colors::TextAccent);
    DrawLine(static_cast<int>(heroCard.x + 28.0f), static_cast<int>(heroCard.y + 48.0f), static_cast<int>(heroCard.x + heroCard.width - 28.0f), static_cast<int>(heroCard.y + 48.0f), Colors::BgPanelBorder);

    // Quick Stats & Live Preview
    DrawText("DEFORMABLE 2D SOFT-BODY SIMULATION", static_cast<int>(heroCard.x + 28.0f), static_cast<int>(heroCard.y + 68.0f), 13, Colors::TextWhite);
    DrawText("Spring-mass lattice models dynamic squish, wobbles and impact inertia.", static_cast<int>(heroCard.x + 28.0f), static_cast<int>(heroCard.y + 88.0f), 11, Colors::TextDim);

    DrawText("ROGUELIKE DECKBUILDING & MUTATORS", static_cast<int>(heroCard.x + 28.0f), static_cast<int>(heroCard.y + 120.0f), 13, Colors::TextWhite);
    DrawText("Draft game-altering relics: Jelly Body, Midas Touch, Fission and more.", static_cast<int>(heroCard.x + 28.0f), static_cast<int>(heroCard.y + 140.0f), 11, Colors::TextDim);

    // Live preview centerpiece
    Vector2 previewCenter = { heroCard.x + heroCard.width * 0.5f, heroCard.y + 320.0f };
    m_menuRenderer.DrawMinoSkinPreview(TetrominoType::T, previewCenter, 38.0f, m_animTimer, Colors::PieceT, Colors::PieceI);

    // Prompt inside hero box
    Rectangle startPill = { heroCard.x + 40.0f, heroCard.y + heroCard.height - 65.0f, heroCard.width - 80.0f, 44.0f };
    bool startHover = CheckCollisionPointRec(mousePos, startPill);
    DrawRectangleRounded(startPill, 0.2f, 4, startHover ? Fade(Colors::TextGreen, 0.3f) : Fade(Colors::TextGreen, 0.15f));
    DrawRectangleLinesEx(startPill, 1.5f, Colors::TextGreen);
    const char* startPrompt = "PRESS [ENTER] OR CLICK TO COMMENCE RUN";
    int spW = MeasureText(startPrompt, 13);
    DrawText(startPrompt, static_cast<int>(startPill.x + (startPill.width - spW) * 0.5f), static_cast<int>(startPill.y + 15.0f), 13, Colors::TextGreen);

    if (startHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SetView(MenuView::PlaySelect, app);
    }

    // 5. Footer Hints
    m_menuRenderer.DrawFooterHints({ "[ARROWS / WASD] Navigate", "[ENTER / SPACE] Select", "[MOUSE] Point & Click", "[ESC] Exit to Desktop" });
}

void TitleState::RenderPlaySelect(GameApp& app) {
    Vector2 mousePos = GetMousePosition();
    m_menuRenderer.DrawHeaderBanner("MISSION SELECTION", "LAUNCH", "PLAY");

    Rectangle card1 = { 100.0f, 130.0f, 500.0f, 500.0f };
    Rectangle card2 = { 680.0f, 130.0f, 500.0f, 500.0f };

    bool hover1 = CheckCollisionPointRec(mousePos, card1);
    bool hover2 = CheckCollisionPointRec(mousePos, card2);

    if (hover1 && m_selectedOption != 0) { m_selectedOption = 0; app.GetSoundSynth().PlayMenuHover(); }
    if (hover2 && m_selectedOption != 1) { m_selectedOption = 1; app.GetSoundSynth().PlayMenuHover(); }

    if (hover1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        app.GetSoundSynth().PlayCardSelect();
        app.GetStateManager().SetState(app, std::make_unique<PlayState>());
        return;
    }
    if (hover2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        app.GetSoundSynth().PlayCardSelect();
        app.GetStateManager().SetState(app, std::make_unique<PlayState>());
        return;
    }

    // Card 1: Roguelike Standard
    Color b1 = (m_selectedOption == 0) ? Colors::PieceI : (hover1 ? WHITE : Colors::BgPanelBorder);
    DrawRectangleRounded(card1, 0.04f, 6, (m_selectedOption == 0) ? Fade(Colors::PieceI, 0.15f) : Colors::BgPanel);
    DrawRectangleLinesEx(card1, (m_selectedOption == 0) ? 2.5f : 1.2f, b1);

    DrawText("SECTOR CAMPAIGN", static_cast<int>(card1.x + 28.0f), static_cast<int>(card1.y + 28.0f), 22, Colors::TextWhite);
    DrawText("ROGUELIKE RUN // PROGRESSION", static_cast<int>(card1.x + 28.0f), static_cast<int>(card1.y + 56.0f), 12, Colors::TextAccent);
    DrawLine(static_cast<int>(card1.x + 28.0f), static_cast<int>(card1.y + 78.0f), static_cast<int>(card1.x + card1.width - 28.0f), static_cast<int>(card1.y + 78.0f), Colors::BgPanelBorder);

    DrawText("* Progress through progressive floors & clear targets", static_cast<int>(card1.x + 28.0f), static_cast<int>(card1.y + 100.0f), 13, Colors::TextWhite);
    DrawText("* Draft powerful Relic Cards after each floor goal", static_cast<int>(card1.x + 28.0f), static_cast<int>(card1.y + 130.0f), 13, Colors::TextWhite);
    DrawText("* Active abilities, physics mutators and score combos", static_cast<int>(card1.x + 28.0f), static_cast<int>(card1.y + 160.0f), 13, Colors::TextWhite);
    DrawText("* High replayability with random 7-bag seeded drops", static_cast<int>(card1.x + 28.0f), static_cast<int>(card1.y + 190.0f), 13, Colors::TextWhite);

    Vector2 c1Center = { card1.x + card1.width * 0.5f, card1.y + 320.0f };
    m_menuRenderer.DrawMinoSkinPreview(TetrominoType::Z, c1Center, 32.0f, m_animTimer, Colors::PieceZ, Colors::PieceI);

    Rectangle btnLaunch1 = { card1.x + 28.0f, card1.y + card1.height - 65.0f, card1.width - 56.0f, 44.0f };
    m_menuRenderer.DrawNeonButton(btnLaunch1, "LAUNCH ROGUELIKE RUN", "[ENTER]", (m_selectedOption == 0), hover1, Colors::TextGreen);

    // Card 2: Endless Sandbox
    Color b2 = (m_selectedOption == 1) ? Colors::PieceT : (hover2 ? WHITE : Colors::BgPanelBorder);
    DrawRectangleRounded(card2, 0.04f, 6, (m_selectedOption == 1) ? Fade(Colors::PieceT, 0.15f) : Colors::BgPanel);
    DrawRectangleLinesEx(card2, (m_selectedOption == 1) ? 2.5f : 1.2f, b2);

    DrawText("ENDLESS MATRIX", static_cast<int>(card2.x + 28.0f), static_cast<int>(card2.y + 28.0f), 22, Colors::TextWhite);
    DrawText("CLASSIC ARCADE // HIGH SCORE CHASE", static_cast<int>(card2.x + 28.0f), static_cast<int>(card2.y + 56.0f), 12, Colors::PieceT);
    DrawLine(static_cast<int>(card2.x + 28.0f), static_cast<int>(card2.y + 78.0f), static_cast<int>(card2.x + card2.width - 28.0f), static_cast<int>(card2.y + 78.0f), Colors::BgPanelBorder);

    DrawText("* Infinite speed-scaling marathon with SRS rotation", static_cast<int>(card2.x + 28.0f), static_cast<int>(card2.y + 100.0f), 13, Colors::TextWhite);
    DrawText("* Soft-body collision squish and wall-kick physics", static_cast<int>(card2.x + 28.0f), static_cast<int>(card2.y + 130.0f), 13, Colors::TextWhite);
    DrawText("* Pure arcade skill: DAS, ARR, Hold and Hard Drop", static_cast<int>(card2.x + 28.0f), static_cast<int>(card2.y + 160.0f), 13, Colors::TextWhite);
    DrawText("* Climb local and global hall of fame leaderboards", static_cast<int>(card2.x + 28.0f), static_cast<int>(card2.y + 190.0f), 13, Colors::TextWhite);

    Vector2 c2Center = { card2.x + card2.width * 0.5f, card2.y + 320.0f };
    m_menuRenderer.DrawMinoSkinPreview(TetrominoType::I, c2Center, 32.0f, m_animTimer, Colors::PieceI, Colors::PieceT);

    Rectangle btnLaunch2 = { card2.x + 28.0f, card2.y + card2.height - 65.0f, card2.width - 56.0f, 44.0f };
    m_menuRenderer.DrawNeonButton(btnLaunch2, "LAUNCH ENDLESS MATRIX", "[ENTER]", (m_selectedOption == 1), hover2, Colors::PieceT);

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[LEFT / RIGHT] Select Mode", "[ENTER / CLICK] Launch Session" });
}

void TitleState::RenderProfileSaves(GameApp& app) {
    Vector2 mousePos = GetMousePosition();
    m_menuRenderer.DrawHeaderBanner("PILOT DOSSIER & SAVE MATRIX", "PROFILE", "DATA");

    // Left Column: Dossier Card
    Rectangle dossierCard = { 60.0f, 110.0f, 380.0f, 570.0f };
    DrawRectangleRounded(dossierCard, 0.04f, 6, Colors::BgPanel);
    DrawRectangleLinesEx(dossierCard, 1.5f, Colors::BgPanelBorder);

    DrawText("PILOT CREDENTIALS", static_cast<int>(dossierCard.x + 20.0f), static_cast<int>(dossierCard.y + 20.0f), 15, Colors::TextAccent);
    DrawLine(static_cast<int>(dossierCard.x + 20.0f), static_cast<int>(dossierCard.y + 44.0f), static_cast<int>(dossierCard.x + dossierCard.width - 20.0f), static_cast<int>(dossierCard.y + 44.0f), Colors::BgPanelBorder);

    int dy = static_cast<int>(dossierCard.y + 60.0f);
    DrawText("CALLSIGN:", static_cast<int>(dossierCard.x + 20.0f), dy, 11, Colors::TextDim);
    DrawText(m_profile.pilotCallsign.c_str(), static_cast<int>(dossierCard.x + 20.0f), dy + 15, 17, Colors::TextWhite); dy += 45;

    DrawText("STATUS & RANK:", static_cast<int>(dossierCard.x + 20.0f), dy, 11, Colors::TextDim);
    DrawText(m_profile.rankTitle.c_str(), static_cast<int>(dossierCard.x + 20.0f), dy + 15, 14, Colors::TextGreen); dy += 45;

    DrawText("TOTAL MISSIONS DEPLOYED:", static_cast<int>(dossierCard.x + 20.0f), dy, 11, Colors::TextDim);
    std::string runsStr = std::to_string(m_profile.totalRuns) + " RUNS (" + std::to_string(m_profile.totalVictories) + " WINS)";
    DrawText(runsStr.c_str(), static_cast<int>(dossierCard.x + 20.0f), dy + 15, 14, Colors::TextWhite); dy += 45;

    DrawText("ALL-TIME LINES CLEARED:", static_cast<int>(dossierCard.x + 20.0f), dy, 11, Colors::TextDim);
    std::string linesStr = std::to_string(m_profile.totalLinesCleared) + " LINES";
    DrawText(linesStr.c_str(), static_cast<int>(dossierCard.x + 20.0f), dy + 15, 14, Colors::PieceI); dy += 45;

    DrawText("HIGHEST SECTOR REACHED:", static_cast<int>(dossierCard.x + 20.0f), dy, 11, Colors::TextDim);
    std::string flStr = "FLOOR " + std::to_string(m_profile.highestFloor);
    DrawText(flStr.c_str(), static_cast<int>(dossierCard.x + 20.0f), dy + 15, 14, Colors::PieceGold); dy += 45;

    DrawText("ENERGY CREDITS BALANCE:", static_cast<int>(dossierCard.x + 20.0f), dy, 11, Colors::TextDim);
    std::string credStr = "$" + std::to_string(m_profile.energyCredits) + " CR";
    DrawText(credStr.c_str(), static_cast<int>(dossierCard.x + 20.0f), dy + 15, 18, Colors::TextGold);

    // Right Column: Save Slots
    float slotX = 470.0f;
    float slotY = 110.0f;
    float slotW = WINDOW_WIDTH - 530.0f;
    float slotH = 175.0f;

    for (size_t i = 0; i < m_saveSlots.size(); ++i) {
        Rectangle sRect = { slotX, slotY + static_cast<float>(i) * (slotH + 16.0f), slotW, slotH };
        bool isHovered = CheckCollisionPointRec(mousePos, sRect);
        bool isSelected = (m_selectedOption == static_cast<int>(i));

        if (isHovered && !isSelected) {
            m_selectedOption = static_cast<int>(i);
            app.GetSoundSynth().PlayMenuHover();
        }

        if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            app.GetSoundSynth().PlayCardSelect();
            app.GetStateManager().SetState(app, std::make_unique<PlayState>());
            return;
        }

        m_menuRenderer.DrawSaveSlotCard(sRect, m_saveSlots[i], isSelected, isHovered);
    }

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[UP / DOWN] Select Slot", "[ENTER / CLICK] Resume / Create Mission" });
}

void TitleState::RenderHighScores(GameApp& /*app*/) {
    Vector2 mousePos = GetMousePosition();
    m_menuRenderer.DrawHeaderBanner("HALL OF FAME // RECORDS", "LEADERBOARD", "SCORES");

    // Filter Tabs
    std::vector<std::string> tabs = { "ALL-TIME MARATHON", "DAILY PROTOCOL", "BOSS RUSH" };
    Rectangle tabRect = { 60.0f, 110.0f, 540.0f, 38.0f };
    m_menuRenderer.DrawTabHeader(tabs, m_activeTab, tabRect);

    // Table Header
    Rectangle tableHeader = { 60.0f, 160.0f, WINDOW_WIDTH - 120.0f, 32.0f };
    DrawRectangleRounded(tableHeader, 0.05f, 4, Colors::GridBg);
    DrawText("RANK", static_cast<int>(tableHeader.x + 16.0f), static_cast<int>(tableHeader.y + 9.0f), 11, Colors::TextDim);
    DrawText("PILOT CALLSIGN", static_cast<int>(tableHeader.x + 80.0f), static_cast<int>(tableHeader.y + 9.0f), 11, Colors::TextDim);
    DrawText("FINAL SCORE", static_cast<int>(tableHeader.x + 280.0f), static_cast<int>(tableHeader.y + 9.0f), 11, Colors::TextDim);
    DrawText("SECTOR", static_cast<int>(tableHeader.x + 440.0f), static_cast<int>(tableHeader.y + 9.0f), 11, Colors::TextDim);
    DrawText("LINES", static_cast<int>(tableHeader.x + 550.0f), static_cast<int>(tableHeader.y + 9.0f), 11, Colors::TextDim);
    DrawText("BADGE TIER", static_cast<int>(tableHeader.x + tableHeader.width - 130.0f), static_cast<int>(tableHeader.y + 9.0f), 11, Colors::TextDim);

    // Rows
    float rowY = 200.0f;
    float rowH = 48.0f;
    for (size_t i = 0; i < m_highScores.size(); ++i) {
        Rectangle rRect = { 60.0f, rowY + static_cast<float>(i) * (rowH + 8.0f), WINDOW_WIDTH - 120.0f, rowH };
        bool isHovered = CheckCollisionPointRec(mousePos, rRect);
        m_menuRenderer.DrawScoreRow(rRect, m_highScores[i], (i % 2 == 0), isHovered);
    }

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[TAB / ARROWS] Switch Leaderboard Filter" });
}

void TitleState::RenderShop(GameApp& app) {
    Vector2 mousePos = GetMousePosition();
    m_menuRenderer.DrawHeaderBanner("DATA MARKET // BLACK MARKET", "SHOP", "STORE");

    // Balance capsule on top right
    Rectangle balancePill = { WINDOW_WIDTH - 240.0f, 36.0f, 180.0f, 40.0f };
    DrawRectangleRounded(balancePill, 0.25f, 4, Fade(Colors::PieceGold, 0.15f));
    DrawRectangleLinesEx(balancePill, 1.2f, Colors::PieceGold);
    DrawText("CREDITS:", static_cast<int>(balancePill.x + 14.0f), static_cast<int>(balancePill.y + 13.0f), 12, Colors::TextDim);
    std::string credStr = "$" + std::to_string(m_profile.energyCredits);
    DrawText(credStr.c_str(), static_cast<int>(balancePill.x + 85.0f), static_cast<int>(balancePill.y + 11.0f), 16, Colors::TextGold);

    // 2x3 Grid of shop items
    float gridX = 60.0f;
    float gridY = 110.0f;
    float itemW = (WINDOW_WIDTH - 150.0f) * 0.5f;
    float itemH = 160.0f;

    for (size_t i = 0; i < m_shopItems.size(); ++i) {
        int col = static_cast<int>(i % 2);
        int row = static_cast<int>(i / 2);
        Rectangle iRect = { gridX + static_cast<float>(col) * (itemW + 30.0f), gridY + static_cast<float>(row) * (itemH + 20.0f), itemW, itemH };

        bool isHovered = CheckCollisionPointRec(mousePos, iRect);
        bool isSelected = (m_selectedOption == static_cast<int>(i));

        if (isHovered && !isSelected) {
            m_selectedOption = static_cast<int>(i);
            app.GetSoundSynth().PlayMenuHover();
        }

        if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            auto& item = m_shopItems[i];
            if (!item.isUnlocked) {
                if (m_profile.energyCredits >= item.cost) {
                    m_profile.energyCredits -= item.cost;
                    item.isUnlocked = true;
                    item.isEquipped = true;
                    app.GetSoundSynth().PlayLevelUp();
                } else {
                    app.GetSoundSynth().PlayMenuBack();
                }
            } else {
                item.isEquipped = !item.isEquipped;
                app.GetSoundSynth().PlayMenuToggle();
            }
        }

        m_menuRenderer.DrawShopItemCard(iRect, m_shopItems[i], m_profile.energyCredits, isSelected, isHovered);
    }

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[UP / DOWN] Select Item", "[ENTER / CLICK] Buy or Equip Item" });
}

void TitleState::RenderCustomization(GameApp& app) {
    Vector2 mousePos = GetMousePosition();
    m_menuRenderer.DrawHeaderBanner("HANGAR // TETROMINO THEMES", "CUSTOMIZE", "HANGAR");

    // Left List: Themes
    float startY = 110.0f;
    float cardW = 540.0f;
    float cardH = 115.0f;

    for (size_t i = 0; i < m_customThemes.size(); ++i) {
        Rectangle tRect = { 60.0f, startY + static_cast<float>(i) * (cardH + 18.0f), cardW, cardH };
        bool isHovered = CheckCollisionPointRec(mousePos, tRect);
        bool isSelected = (m_selectedOption == static_cast<int>(i));

        if (isHovered && !isSelected) {
            m_selectedOption = static_cast<int>(i);
            m_selectedThemeIndex = static_cast<int>(i);
            app.GetSoundSynth().PlayMenuHover();
        }

        if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (size_t j = 0; j < m_customThemes.size(); ++j) {
                m_customThemes[j].isEquipped = (j == i);
            }
            app.GetSoundSynth().PlayCardSelect();
        }

        Color border = isSelected ? m_customThemes[i].primaryColor : (isHovered ? WHITE : Colors::BgPanelBorder);
        Color bg = isSelected ? Fade(m_customThemes[i].primaryColor, 0.15f) : Colors::BgPanel;

        DrawRectangleRounded(tRect, 0.08f, 6, bg);
        DrawRectangleLinesEx(tRect, isSelected ? 2.5f : 1.2f, border);

        DrawText(m_customThemes[i].name.c_str(), static_cast<int>(tRect.x + 18.0f), static_cast<int>(tRect.y + 16.0f), 17, Colors::TextWhite);
        DrawText(m_customThemes[i].description.c_str(), static_cast<int>(tRect.x + 18.0f), static_cast<int>(tRect.y + 44.0f), 11, Colors::TextDim);

        if (m_customThemes[i].isEquipped) {
            DrawText("[EQUIPPED / ACTIVE]", static_cast<int>(tRect.x + 18.0f), static_cast<int>(tRect.y + 80.0f), 12, Colors::TextGreen);
        } else {
            DrawText("CLICK TO EQUIP", static_cast<int>(tRect.x + 18.0f), static_cast<int>(tRect.y + 80.0f), 12, Colors::TextDim);
        }
    }

    // Right Box: Live Interactive Preview Chamber
    Rectangle chamberRect = { 650.0f, 110.0f, WINDOW_WIDTH - 710.0f, 520.0f };
    DrawRectangleRounded(chamberRect, 0.04f, 6, Colors::BgPanel);
    DrawRectangleLinesEx(chamberRect, 1.5f, Colors::BgPanelBorder);

    DrawText("LIVE SIMULATION CHAMBER", static_cast<int>(chamberRect.x + 24.0f), static_cast<int>(chamberRect.y + 20.0f), 14, Colors::TextAccent);
    DrawLine(static_cast<int>(chamberRect.x + 24.0f), static_cast<int>(chamberRect.y + 44.0f), static_cast<int>(chamberRect.x + chamberRect.width - 24.0f), static_cast<int>(chamberRect.y + 44.0f), Colors::BgPanelBorder);

    if (m_selectedThemeIndex >= 0 && m_selectedThemeIndex < static_cast<int>(m_customThemes.size())) {
        const auto& theme = m_customThemes[m_selectedThemeIndex];
        Vector2 previewPos = { chamberRect.x + chamberRect.width * 0.5f, chamberRect.y + chamberRect.height * 0.45f };
        m_menuRenderer.DrawMinoSkinPreview(theme.previewType, previewPos, 44.0f, m_animTimer, theme.primaryColor, theme.secondaryColor);

        int textW = MeasureText(theme.name.c_str(), 16);
        DrawText(theme.name.c_str(), static_cast<int>(chamberRect.x + (chamberRect.width - textW) * 0.5f), static_cast<int>(chamberRect.y + chamberRect.height - 80.0f), 16, Colors::TextWhite);
        const char* note = "Real-time Verlet spring lattice preview";
        int nw = MeasureText(note, 11);
        DrawText(note, static_cast<int>(chamberRect.x + (chamberRect.width - nw) * 0.5f), static_cast<int>(chamberRect.y + chamberRect.height - 55.0f), 11, Colors::TextDim);
    }

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[UP / DOWN] Select Skin", "[ENTER / CLICK] Equip Theme" });
}

void TitleState::RenderSettings(GameApp& app) {
    Vector2 mousePos = GetMousePosition();
    m_menuRenderer.DrawHeaderBanner("SYSTEM CONFIGURATION", "SETTINGS", "CONFIG");

    float startY = 110.0f;
    float rowW = WINDOW_WIDTH - 120.0f;
    float rowH = 64.0f;
    float spacing = 16.0f;

    // 1. Master Audio Volume Slider
    Rectangle r0 = { 60.0f, startY, rowW, rowH };
    bool h0 = CheckCollisionPointRec(mousePos, r0);
    if (h0 && m_selectedOption != 0) { m_selectedOption = 0; app.GetSoundSynth().PlayMenuHover(); }
    if (h0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float norm = (mousePos.x - (r0.x + 220.0f)) / (r0.width - 340.0f);
        m_settings.masterVolume = std::min(1.0f, std::max(0.0f, norm));
        app.GetSoundSynth().SetMasterVolume(m_settings.masterVolume);
    }
    char volBuffer[16];
    snprintf(volBuffer, sizeof(volBuffer), "%d %%", static_cast<int>(m_settings.masterVolume * 100.0f));
    m_menuRenderer.DrawSlider(r0, "MASTER AUDIO VOLUME", m_settings.masterVolume, 0.0f, 1.0f, volBuffer, (m_selectedOption == 0));

    // 2. Screen Shake Intensity
    Rectangle r1 = { 60.0f, startY + (rowH + spacing), rowW, rowH };
    bool h1 = CheckCollisionPointRec(mousePos, r1);
    if (h1 && m_selectedOption != 1) { m_selectedOption = 1; app.GetSoundSynth().PlayMenuHover(); }
    if (h1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.screenShakeLevel = (m_settings.screenShakeLevel + 1) % 4;
        app.GetSoundSynth().PlayMenuToggle();
    }
    const char* shakeLabels[4] = { "DISABLED (0x)", "SUBTLE (0.5x)", "STANDARD (1.0x)", "INTENSE (1.5x)" };
    m_menuRenderer.DrawSlider(r1, "IMPACT SCREEN SHAKE", static_cast<float>(m_settings.screenShakeLevel), 0.0f, 3.0f, shakeLabels[m_settings.screenShakeLevel], (m_selectedOption == 1));

    // 3. CRT Scanlines & Bloom Toggle
    Rectangle r2 = { 60.0f, startY + 2.0f * (rowH + spacing), rowW, rowH };
    bool h2 = CheckCollisionPointRec(mousePos, r2);
    if (h2 && m_selectedOption != 2) { m_selectedOption = 2; app.GetSoundSynth().PlayMenuHover(); }
    if (h2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.crtScanlines = !m_settings.crtScanlines;
        app.GetSoundSynth().PlayMenuToggle();
    }
    m_menuRenderer.DrawToggle(r2, "POST-PROCESS CRT SCANLINES & BLOOM", m_settings.crtScanlines, (m_selectedOption == 2));

    // 4. Soft-Body Wobble Intensity
    Rectangle r3 = { 60.0f, startY + 3.0f * (rowH + spacing), rowW, rowH };
    bool h3 = CheckCollisionPointRec(mousePos, r3);
    if (h3 && m_selectedOption != 3) { m_selectedOption = 3; app.GetSoundSynth().PlayMenuHover(); }
    if (h3 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.softBodyWobble = (m_settings.softBodyWobble + 1) % 3;
        app.GetSoundSynth().PlayMenuToggle();
    }
    const char* wobbleLabels[3] = { "RELAXED (MILD SQUISH)", "STANDARD (SPRING LATTICE)", "ULTRA JELLY (MAX ELASTICITY)" };
    m_menuRenderer.DrawSlider(r3, "SOFT-BODY DEFORMATION DEGREE", static_cast<float>(m_settings.softBodyWobble), 0.0f, 2.0f, wobbleLabels[m_settings.softBodyWobble], (m_selectedOption == 3));

    // 5. Fast DAS / ARR Handling
    Rectangle r4 = { 60.0f, startY + 4.0f * (rowH + spacing), rowW, rowH };
    bool h4 = CheckCollisionPointRec(mousePos, r4);
    if (h4 && m_selectedOption != 4) { m_selectedOption = 4; app.GetSoundSynth().PlayMenuHover(); }
    if (h4 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.fastDAS = !m_settings.fastDAS;
        app.GetSoundSynth().PlayMenuToggle();
    }
    m_menuRenderer.DrawToggle(r4, "TOURNAMENT DAS & ARR RESPONSE (HIGH REPEAT)", m_settings.fastDAS, (m_selectedOption == 4));

    // Keybindings Quick Overview Box
    Rectangle kbBox = { 60.0f, startY + 5.0f * (rowH + spacing), rowW, 110.0f };
    DrawRectangleRounded(kbBox, 0.08f, 4, Colors::BgPanel);
    DrawRectangleLinesEx(kbBox, 1.0f, Colors::BgPanelBorder);
    DrawText("DEFAULT INPUT BINDINGS:", static_cast<int>(kbBox.x + 16.0f), static_cast<int>(kbBox.y + 14.0f), 12, Colors::TextAccent);
    DrawText("[LEFT / RIGHT / DOWN] Move & Soft Drop   |   [UP / X] Rotate CW   |   [Z] Rotate CCW", static_cast<int>(kbBox.x + 16.0f), static_cast<int>(kbBox.y + 36.0f), 12, Colors::TextWhite);
    DrawText("[SPACE] Hard Drop   |   [C / SHIFT] Hold Piece   |   [1 / 2] Trigger Active Relic Cards", static_cast<int>(kbBox.x + 16.0f), static_cast<int>(kbBox.y + 58.0f), 12, Colors::TextWhite);
    DrawText("[P] Pause Game   |   [F1] Toggle Verlet Springs Debug   |   [F2] Card Draft Test", static_cast<int>(kbBox.x + 16.0f), static_cast<int>(kbBox.y + 80.0f), 12, Colors::TextDim);

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[UP / DOWN] Select Setting", "[LEFT / RIGHT / CLICK] Adjust Setting" });
}

void TitleState::RenderQuitConfirm(GameApp& app) {
    Vector2 mousePos = GetMousePosition();

    Rectangle modalRect = { WINDOW_WIDTH * 0.5f - 240.0f, WINDOW_HEIGHT * 0.5f - 110.0f, 480.0f, 220.0f };
    m_menuRenderer.DrawModalFrame(modalRect, "DISENGAGE MISSION PROTOCOL");

    const char* question = "Are you sure you want to terminate session and exit to desktop?";
    DrawText(question, static_cast<int>(modalRect.x + 24.0f), static_cast<int>(modalRect.y + 65.0f), 13, Colors::TextWhite);

    Rectangle btnCancel = { modalRect.x + 30.0f, modalRect.y + 135.0f, 195.0f, 48.0f };
    Rectangle btnConfirm = { modalRect.x + 255.0f, modalRect.y + 135.0f, 195.0f, 48.0f };

    bool hoverCancel = CheckCollisionPointRec(mousePos, btnCancel);
    bool hoverConfirm = CheckCollisionPointRec(mousePos, btnConfirm);

    if (hoverCancel && m_selectedOption != 0) { m_selectedOption = 0; app.GetSoundSynth().PlayMenuHover(); }
    if (hoverConfirm && m_selectedOption != 1) { m_selectedOption = 1; app.GetSoundSynth().PlayMenuHover(); }

    if (hoverCancel && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SetView(MenuView::Main, app);
        return;
    }
    if (hoverConfirm && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        CloseWindow();
        return;
    }

    m_menuRenderer.DrawNeonButton(btnCancel, "RESUME [ESC]", nullptr, (m_selectedOption == 0), hoverCancel, Colors::TextGreen);
    m_menuRenderer.DrawNeonButton(btnConfirm, "EXIT TO OS", nullptr, (m_selectedOption == 1), hoverConfirm, Colors::PieceBomb);
}

} // namespace TetroShift
