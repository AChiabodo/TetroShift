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

    InitializeRealData(app);
    app.GetSoundSynth().SetMasterVolume(m_settings.masterVolume);
    app.GetMusicManager().SetVolume(m_settings.musicVolume);
    app.GetMusicManager().SetFixedTrackIndex(m_settings.fixedSoundtrack);
    app.GetMusicManager().PlayTrack(TrackId::MenuTheme, true);
}

void TitleState::OnExit(GameApp& /*app*/) {}

void TitleState::InitializeRealData(GameApp& app) {
    // Default catalog templates if empty
    if (m_shopItems.empty()) {
        m_shopItems = {
            { "SHOP_JELLY_V2", "SUPER JELLY COATING", ShopCategory::Relic, "RELIC // PASSIVE", "+30% spring bounce and self-stabilizing lock", 350, false, false, Colors::PieceJelly },
            { "SHOP_START_COINS", "MIDAS CATALYST", ShopCategory::Booster, "BOOSTER // ACTIVE", "Start every new run with +100 Energy Credits", 500, false, false, Colors::PieceGold },
            { "SHOP_SKIN_CYBER", "CYBERPUNK NEON SKIN", ShopCategory::Skin, "COSMETIC // THEME", "High-glow chromatic aberration block theme", 400, true, true, Colors::PieceI },
            { "SHOP_SKIN_TITAN", "TITAN METALLIC SKIN", ShopCategory::Skin, "COSMETIC // THEME", "Heavy brushed titanium aesthetics with sparks", 650, false, false, Colors::PieceIron },
            { "SHOP_REROLL_PACK", "QUANTUM REROLL MATRIX", ShopCategory::Booster, "BOOSTER // PERK", "+2 Draft card reroll tokens per run", 450, false, false, Colors::PieceT },
            { "SHOP_CRT_PULSE", "SYNTHWAVE GRID THEME", ShopCategory::MatrixTheme, "COSMETIC // MATRIX", "Undulating retro-futuristic horizon matrix grid", 300, true, false, Colors::PieceZ }
        };
    }

    if (m_customThemes.empty()) {
        m_customThemes = {
            { "THEME_NEON", "CYBER NEON (DEFAULT)", "High-contrast electric neon with cyan and violet accents", Colors::PieceI, Colors::PieceT, TetrominoType::T, true, true },
            { "THEME_TITAN", "TITAN INDUSTRIAL", "Heavy metallic frames with intense industrial orange glow", Colors::PieceIron, Colors::PieceL, TetrominoType::I, false, false },
            { "THEME_JELLY", "GELATIN LUMINESCENCE", "Soft aquamarine translucent blocks with harmonic wobble", Colors::PieceJelly, Colors::PieceS, TetrominoType::S, true, false },
            { "THEME_RETRO", "ARCADE SYNTH 1984", "Classic vibrant primaries with retro crt phosphor edge", Colors::PieceO, Colors::PieceBomb, TetrominoType::L, false, false }
        };
    }

    std::string equippedTheme = "THEME_NEON";
    app.GetSaveManager().LoadProfile(m_profile, m_settings, m_shopItems, equippedTheme);
    m_highScores = app.GetSaveManager().LoadHighScores();
    m_saveSlots = app.GetSaveManager().GetSaveSlotHeaders();

    for (size_t i = 0; i < m_customThemes.size(); ++i) {
        if (m_customThemes[i].id == equippedTheme) {
            m_customThemes[i].isEquipped = true;
            m_selectedThemeIndex = static_cast<int>(i);
        } else {
            m_customThemes[i].isEquipped = false;
        }
    }
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

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        if (m_selectedOption % 2 == 1) m_selectedOption -= 1;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        if (m_selectedOption % 2 == 0) m_selectedOption += 1;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        if (m_selectedOption >= 2) m_selectedOption -= 2;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        if (m_selectedOption < 2) m_selectedOption += 2;
        app.GetSoundSynth().PlayMenuHover();
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        app.GetSoundSynth().PlayCardSelect();
        switch (m_selectedOption) {
            case 0: // Roguelike
                app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::Roguelike, 1, std::nullopt));
                break;
            case 1: // Marathon
                app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::Marathon, 1, std::nullopt));
                break;
            case 2: // Daily Protocol
                app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::DailyProtocol, 1, std::nullopt, SaveManager::ComputeDailySeed()));
                break;
            case 3: // Sandbox
                app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::Sandbox, 1, std::nullopt));
                break;
        }
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

    // Wipe Slot with DEL or X
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_X)) {
        int slotId = m_selectedOption + 1;
        app.GetSaveManager().DeleteRunSlot(slotId);
        m_saveSlots = app.GetSaveManager().GetSaveSlotHeaders();
        app.GetSoundSynth().PlayMenuBack();
        return;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        app.GetSoundSynth().PlayCardSelect();
        int slotId = m_selectedOption + 1;
        SavedRunState saved;
        if (app.GetSaveManager().LoadRunSlot(slotId, saved)) {
            app.GetStateManager().SetState(app, std::make_unique<PlayState>(saved.gameMode, slotId, saved));
        } else {
            app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::Roguelike, slotId, std::nullopt));
        }
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

                    std::string eqTheme = "THEME_NEON";
                    for (const auto& t : m_customThemes) if (t.isEquipped) eqTheme = t.id;
                    app.GetSaveManager().SaveProfile(m_profile, m_settings, m_shopItems, eqTheme);
                } else {
                    app.GetSoundSynth().PlayMenuBack();
                }
            } else {
                item.isEquipped = !item.isEquipped;
                app.GetSoundSynth().PlayMenuToggle();
                std::string eqTheme = "THEME_NEON";
                for (const auto& t : m_customThemes) if (t.isEquipped) eqTheme = t.id;
                app.GetSaveManager().SaveProfile(m_profile, m_settings, m_shopItems, eqTheme);
            }
        }
    }
}

void TitleState::HandleInputCustomization(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE)) { GoBack(app); return; }

    // Tab switching with TAB
    if (IsKeyPressed(KEY_TAB)) {
        m_activeTab = (m_activeTab == 0) ? 1 : 0;
        m_selectedOption = 0;
        app.GetSoundSynth().PlayMenuToggle();
        return;
    }

    if (m_activeTab == 0) {
        // Tab 0: Mino Visual Skins
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
            std::string eqTheme = m_customThemes[m_selectedOption].id;
            app.GetSaveManager().SaveProfile(m_profile, m_settings, m_shopItems, eqTheme);
        }
    } else {
        // Tab 1: Soundtrack Jukebox & Fixed Track Selection
        const auto& catalog = app.GetMusicManager().GetTrackCatalog();
        const int numTracks = static_cast<int>(catalog.size());

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            m_selectedJukeboxIndex = (m_selectedJukeboxIndex - 1 + numTracks) % numTracks;
            app.GetSoundSynth().PlayMenuHover();
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            m_selectedJukeboxIndex = (m_selectedJukeboxIndex + 1) % numTracks;
            app.GetSoundSynth().PlayMenuHover();
        }

        // Preview track with SPACE
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_P)) {
            if (m_selectedJukeboxIndex >= 0 && m_selectedJukeboxIndex < numTracks) {
                app.GetMusicManager().PlayTrack(catalog[m_selectedJukeboxIndex].id, true);
                app.GetSoundSynth().PlayMenuToggle();
            }
        }

        // Lock/Unlock fixed soundtrack with ENTER
        if (IsKeyPressed(KEY_ENTER)) {
            if (m_selectedJukeboxIndex >= 0 && m_selectedJukeboxIndex < numTracks) {
                int newFixed = m_selectedJukeboxIndex + 1;
                if (m_settings.fixedSoundtrack == newFixed) {
                    m_settings.fixedSoundtrack = 0; // Dynamic
                    app.GetSoundSynth().PlayMenuBack();
                } else {
                    m_settings.fixedSoundtrack = newFixed;
                    app.GetSoundSynth().PlayLevelUp();
                }
                app.GetMusicManager().SetFixedTrackIndex(m_settings.fixedSoundtrack);
                app.GetMusicManager().PlayTrack(catalog[m_selectedJukeboxIndex].id, true);

                std::string eqTheme = "THEME_NEON";
                for (const auto& t : m_customThemes) if (t.isEquipped) eqTheme = t.id;
                app.GetSaveManager().SaveProfile(m_profile, m_settings, m_shopItems, eqTheme);
            }
        }
    }
}

void TitleState::HandleInputSettings(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE)) { GoBack(app); return; }

    const int numSettings = 8;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + numSettings) % numSettings;
        app.GetSoundSynth().PlayMenuHover();
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % numSettings;
        app.GetSoundSynth().PlayMenuHover();
    }

    const auto& catalog = app.GetMusicManager().GetTrackCatalog();

    // Left adjustments
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        if (m_selectedOption == 0) { // Master Volume
            m_settings.masterVolume = std::max(0.0f, m_settings.masterVolume - 0.1f);
            app.GetSoundSynth().SetMasterVolume(m_settings.masterVolume);
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 1) { // Music Volume
            m_settings.musicVolume = std::max(0.0f, m_settings.musicVolume - 0.1f);
            app.GetMusicManager().SetVolume(m_settings.musicVolume);
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 2) { // SFX Volume
            m_settings.sfxVolume = std::max(0.0f, m_settings.sfxVolume - 0.1f);
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 3) { // Fixed Soundtrack Mode
            m_settings.fixedSoundtrack = (m_settings.fixedSoundtrack - 1 + 9) % 9;
            app.GetMusicManager().SetFixedTrackIndex(m_settings.fixedSoundtrack);
            if (m_settings.fixedSoundtrack > 0 && m_settings.fixedSoundtrack <= static_cast<int>(catalog.size())) {
                app.GetMusicManager().PlayTrack(catalog[m_settings.fixedSoundtrack - 1].id, true);
            } else {
                app.GetMusicManager().PlayTrack(TrackId::MenuTheme, true);
            }
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 4) { // Screen Shake
            m_settings.screenShakeLevel = (m_settings.screenShakeLevel - 1 + 4) % 4;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 5) { // CRT Scanlines
            m_settings.crtScanlines = !m_settings.crtScanlines;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 6) { // Soft-Body Wobble
            m_settings.softBodyWobble = (m_settings.softBodyWobble - 1 + 3) % 3;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 7) { // Fast DAS
            m_settings.fastDAS = !m_settings.fastDAS;
            app.GetSoundSynth().PlayMenuToggle();
        }
    }
    // Right / Enter adjustments
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (m_selectedOption == 0) { // Master Volume
            m_settings.masterVolume = std::min(1.0f, m_settings.masterVolume + 0.1f);
            app.GetSoundSynth().SetMasterVolume(m_settings.masterVolume);
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 1) { // Music Volume
            m_settings.musicVolume = std::min(1.0f, m_settings.musicVolume + 0.1f);
            app.GetMusicManager().SetVolume(m_settings.musicVolume);
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 2) { // SFX Volume
            m_settings.sfxVolume = std::min(1.0f, m_settings.sfxVolume + 0.1f);
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 3) { // Fixed Soundtrack Mode
            m_settings.fixedSoundtrack = (m_settings.fixedSoundtrack + 1) % 9;
            app.GetMusicManager().SetFixedTrackIndex(m_settings.fixedSoundtrack);
            if (m_settings.fixedSoundtrack > 0 && m_settings.fixedSoundtrack <= static_cast<int>(catalog.size())) {
                app.GetMusicManager().PlayTrack(catalog[m_settings.fixedSoundtrack - 1].id, true);
            } else {
                app.GetMusicManager().PlayTrack(TrackId::MenuTheme, true);
            }
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 4) { // Screen Shake
            m_settings.screenShakeLevel = (m_settings.screenShakeLevel + 1) % 4;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 5) { // CRT Scanlines
            m_settings.crtScanlines = !m_settings.crtScanlines;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 6) { // Soft-Body Wobble
            m_settings.softBodyWobble = (m_settings.softBodyWobble + 1) % 3;
            app.GetSoundSynth().PlayMenuToggle();
        } else if (m_selectedOption == 7) { // Fast DAS
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
            app.RequestExit();
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

    // 3. HUD Now Playing Banner
    if (app.GetMusicManager().IsNowPlayingVisible()) {
        m_menuRenderer.DrawNowPlayingBanner(
            app.GetMusicManager().GetNowPlayingTitle().c_str(),
            app.GetMusicManager().GetNowPlayingGenre().c_str(),
            app.GetMusicManager().GetNowPlayingAlpha()
        );
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

    Rectangle cards[4] = {
        { 60.0f, 105.0f, 560.0f, 275.0f },
        { 660.0f, 105.0f, 560.0f, 275.0f },
        { 60.0f, 395.0f, 560.0f, 275.0f },
        { 660.0f, 395.0f, 560.0f, 275.0f }
    };

    bool hovers[4];
    for (int i = 0; i < 4; ++i) {
        hovers[i] = CheckCollisionPointRec(mousePos, cards[i]);
        if (hovers[i] && m_selectedOption != i) {
            m_selectedOption = i;
            app.GetSoundSynth().PlayMenuHover();
        }
        if (hovers[i] && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            app.GetSoundSynth().PlayCardSelect();
            switch (i) {
                case 0:
                    app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::Roguelike, 1, std::nullopt));
                    break;
                case 1:
                    app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::Marathon, 1, std::nullopt));
                    break;
                case 2:
                    app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::DailyProtocol, 1, std::nullopt, SaveManager::ComputeDailySeed()));
                    break;
                case 3:
                    app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::Sandbox, 1, std::nullopt));
                    break;
            }
            return;
        }
    }

    // 1. Sector Campaign (Roguelike)
    {
        Rectangle c = cards[0];
        bool sel = (m_selectedOption == 0);
        DrawRectangleRounded(c, 0.04f, 6, sel ? Fade(Colors::PieceI, 0.15f) : Colors::BgPanel);
        DrawRectangleLinesEx(c, sel ? 2.5f : 1.2f, sel ? Colors::PieceI : (hovers[0] ? WHITE : Colors::BgPanelBorder));

        DrawText("SECTOR CAMPAIGN", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 16.0f), 18, Colors::TextWhite);
        DrawText("ROGUELIKE PROGRESSION & DRAFTS", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 40.0f), 11, Colors::TextAccent);
        DrawLine(static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 56.0f), static_cast<int>(c.x + c.width - 20.0f), static_cast<int>(c.y + 56.0f), Colors::BgPanelBorder);

        DrawText("* Floor targets & 30 Relic Cards catalog", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 70.0f), 11, Colors::TextWhite);
        DrawText("* In-run shop, deflector shields & active spells", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 90.0f), 11, Colors::TextWhite);
        DrawText("* Boss Hazards: Gravity Flux, Glitch Matrix", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 110.0f), 11, Colors::TextWhite);

        Vector2 previewCenter = { c.x + c.width - 70.0f, c.y + 105.0f };
        m_menuRenderer.DrawMinoSkinPreview(TetrominoType::Z, previewCenter, 18.0f, m_animTimer, Colors::PieceZ, Colors::PieceI);

        Rectangle btn = { c.x + 20.0f, c.y + c.height - 48.0f, c.width - 40.0f, 36.0f };
        m_menuRenderer.DrawNeonButton(btn, "LAUNCH ROGUELIKE RUN", "[ENTER]", sel, hovers[0], Colors::TextGreen);
    }

    // 2. Matrix Marathon (Classic Arcade)
    {
        Rectangle c = cards[1];
        bool sel = (m_selectedOption == 1);
        DrawRectangleRounded(c, 0.04f, 6, sel ? Fade(Colors::PieceS, 0.15f) : Colors::BgPanel);
        DrawRectangleLinesEx(c, sel ? 2.5f : 1.2f, sel ? Colors::PieceS : (hovers[1] ? WHITE : Colors::BgPanelBorder));

        DrawText("MATRIX MARATHON", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 16.0f), 18, Colors::TextWhite);
        DrawText("PURE CLASSIC ARCADE // LEVEL 1..15", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 40.0f), 11, Colors::PieceS);
        DrawLine(static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 56.0f), static_cast<int>(c.x + c.width - 20.0f), static_cast<int>(c.y + 56.0f), Colors::BgPanelBorder);

        DrawText("* Pure standard SRS mechanics without relics", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 70.0f), 11, Colors::TextWhite);
        DrawText("* 10 lines per level speed scaling curve", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 90.0f), 11, Colors::TextWhite);
        DrawText("* Dedicated Marathon local hall of fame", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 110.0f), 11, Colors::TextWhite);

        Vector2 previewCenter = { c.x + c.width - 70.0f, c.y + 105.0f };
        m_menuRenderer.DrawMinoSkinPreview(TetrominoType::I, previewCenter, 18.0f, m_animTimer, Colors::PieceI, Colors::PieceS);

        Rectangle btn = { c.x + 20.0f, c.y + c.height - 48.0f, c.width - 40.0f, 36.0f };
        m_menuRenderer.DrawNeonButton(btn, "LAUNCH MARATHON", "[ENTER]", sel, hovers[1], Colors::PieceS);
    }

    // 3. Daily Protocol (Seeded Global Run)
    {
        Rectangle c = cards[2];
        bool sel = (m_selectedOption == 2);
        DrawRectangleRounded(c, 0.04f, 6, sel ? Fade(Colors::TextGold, 0.15f) : Colors::BgPanel);
        DrawRectangleLinesEx(c, sel ? 2.5f : 1.2f, sel ? Colors::TextGold : (hovers[2] ? WHITE : Colors::BgPanelBorder));

        DrawText("DAILY PROTOCOL", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 16.0f), 18, Colors::TextWhite);
        std::string dateSub = "SYNCHRONIZED SEED // " + SaveManager::GetDailyDateString();
        DrawText(dateSub.c_str(), static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 40.0f), 11, Colors::TextGold);
        DrawLine(static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 56.0f), static_cast<int>(c.x + c.width - 20.0f), static_cast<int>(c.y + 56.0f), Colors::BgPanelBorder);

        DrawText("* Identical 7-bag piece sequence for all pilots", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 70.0f), 11, Colors::TextWhite);
        DrawText("* Deterministic card draft pool based on daily date", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 90.0f), 11, Colors::TextWhite);
        DrawText("* One global challenge per day to set your high score", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 110.0f), 11, Colors::TextWhite);

        Vector2 previewCenter = { c.x + c.width - 70.0f, c.y + 105.0f };
        m_menuRenderer.DrawMinoSkinPreview(TetrominoType::O, previewCenter, 18.0f, m_animTimer, Colors::PieceGold, Colors::PieceO);

        Rectangle btn = { c.x + 20.0f, c.y + c.height - 48.0f, c.width - 40.0f, 36.0f };
        m_menuRenderer.DrawNeonButton(btn, "LAUNCH DAILY PROTOCOL", "[ENTER]", sel, hovers[2], Colors::TextGold);
    }

    // 4. Custom Sandbox (Training & Physics Testing Lab)
    {
        Rectangle c = cards[3];
        bool sel = (m_selectedOption == 3);
        DrawRectangleRounded(c, 0.04f, 6, sel ? Fade(Colors::PieceSand, 0.15f) : Colors::BgPanel);
        DrawRectangleLinesEx(c, sel ? 2.5f : 1.2f, sel ? Colors::PieceSand : (hovers[3] ? WHITE : Colors::BgPanelBorder));

        DrawText("CUSTOM SANDBOX", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 16.0f), 18, Colors::TextWhite);
        DrawText("PHYSICS & RELIC EXPERIMENTATION LAB", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 40.0f), 11, Colors::PieceSand);
        DrawLine(static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 56.0f), static_cast<int>(c.x + c.width - 20.0f), static_cast<int>(c.y + 56.0f), Colors::BgPanelBorder);

        DrawText("* Spawn any tetromino (1..7) & mino (Sand/Bomb/Jelly)", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 70.0f), 11, Colors::TextWhite);
        DrawText("* Realtime spring elasticity & 0G gravity frozen practice", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 90.0f), 11, Colors::TextWhite);
        DrawText("* Clear matrix (F10), inject garbage lines & test T-Spins", static_cast<int>(c.x + 20.0f), static_cast<int>(c.y + 110.0f), 11, Colors::TextWhite);

        Vector2 previewCenter = { c.x + c.width - 70.0f, c.y + 105.0f };
        m_menuRenderer.DrawMinoSkinPreview(TetrominoType::T, previewCenter, 18.0f, m_animTimer, Colors::PieceSand, Colors::PieceT);

        Rectangle btn = { c.x + 20.0f, c.y + c.height - 48.0f, c.width - 40.0f, 36.0f };
        m_menuRenderer.DrawNeonButton(btn, "LAUNCH SANDBOX LAB", "[ENTER]", sel, hovers[3], Colors::PieceSand);
    }

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[ARROWS / WASD] Select Mode", "[ENTER / CLICK] Launch Session" });
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
            int slotId = static_cast<int>(i) + 1;
            SavedRunState saved;
            if (app.GetSaveManager().LoadRunSlot(slotId, saved)) {
                app.GetStateManager().SetState(app, std::make_unique<PlayState>(saved.gameMode, slotId, saved));
            } else {
                app.GetStateManager().SetState(app, std::make_unique<PlayState>(GameMode::Roguelike, slotId, std::nullopt));
            }
            return;
        }

        m_menuRenderer.DrawSaveSlotCard(sRect, m_saveSlots[i], isSelected, isHovered);
    }

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[UP / DOWN] Select Slot", "[ENTER / CLICK] Resume / Start Mission", "[DEL / X] Wipe Slot" });
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

                    std::string eqTheme = "THEME_NEON";
                    for (const auto& t : m_customThemes) if (t.isEquipped) eqTheme = t.id;
                    app.GetSaveManager().SaveProfile(m_profile, m_settings, m_shopItems, eqTheme);
                } else {
                    app.GetSoundSynth().PlayMenuBack();
                }
            } else {
                item.isEquipped = !item.isEquipped;
                app.GetSoundSynth().PlayMenuToggle();

                std::string eqTheme = "THEME_NEON";
                for (const auto& t : m_customThemes) if (t.isEquipped) eqTheme = t.id;
                app.GetSaveManager().SaveProfile(m_profile, m_settings, m_shopItems, eqTheme);
            }
        }

        m_menuRenderer.DrawShopItemCard(iRect, m_shopItems[i], m_profile.energyCredits, isSelected, isHovered);
    }

    m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[UP / DOWN] Select Item", "[ENTER / CLICK] Buy or Equip Item" });
}

void TitleState::RenderCustomization(GameApp& app) {
    Vector2 mousePos = GetMousePosition();
    m_menuRenderer.DrawHeaderBanner("HANGAR // CUSTOMIZATION & JUKEBOX", "CUSTOMIZE", "HANGAR");

    // Top Tabs
    std::vector<std::string> tabs = { "MINO VISUAL SKINS", "SOUNDTRACK JUKEBOX" };
    Rectangle tabRect = { 60.0f, 95.0f, 540.0f, 38.0f };
    int hoveredTab = -1;
    if (CheckCollisionPointRec(mousePos, { 60.0f, 95.0f, 270.0f, 38.0f })) hoveredTab = 0;
    else if (CheckCollisionPointRec(mousePos, { 330.0f, 95.0f, 270.0f, 38.0f })) hoveredTab = 1;

    if (hoveredTab != -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m_activeTab != hoveredTab) {
        m_activeTab = hoveredTab;
        m_selectedOption = 0;
        app.GetSoundSynth().PlayMenuToggle();
    }

    m_menuRenderer.DrawTabHeader(tabs, m_activeTab, tabRect, hoveredTab);

    float startY = 145.0f;

    if (m_activeTab == 0) {
        // Tab 0: Themes
        float cardW = 540.0f;
        float cardH = 112.0f;

        for (size_t i = 0; i < m_customThemes.size(); ++i) {
            Rectangle tRect = { 60.0f, startY + static_cast<float>(i) * (cardH + 16.0f), cardW, cardH };
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
                std::string eqTheme = m_customThemes[i].id;
                app.GetSaveManager().SaveProfile(m_profile, m_settings, m_shopItems, eqTheme);
            }

            Color border = isSelected ? m_customThemes[i].primaryColor : (isHovered ? WHITE : Colors::BgPanelBorder);
            Color bg = isSelected ? Fade(m_customThemes[i].primaryColor, 0.15f) : Colors::BgPanel;

            DrawRectangleRounded(tRect, 0.08f, 6, bg);
            DrawRectangleLinesEx(tRect, isSelected ? 2.5f : 1.2f, border);

            DrawText(m_customThemes[i].name.c_str(), static_cast<int>(tRect.x + 18.0f), static_cast<int>(tRect.y + 16.0f), 17, Colors::TextWhite);
            DrawText(m_customThemes[i].description.c_str(), static_cast<int>(tRect.x + 18.0f), static_cast<int>(tRect.y + 44.0f), 11, Colors::TextDim);

            if (m_customThemes[i].isEquipped) {
                DrawText("[EQUIPPED / ACTIVE]", static_cast<int>(tRect.x + 18.0f), static_cast<int>(tRect.y + 78.0f), 12, Colors::TextGreen);
            } else {
                DrawText("CLICK TO EQUIP", static_cast<int>(tRect.x + 18.0f), static_cast<int>(tRect.y + 78.0f), 12, Colors::TextDim);
            }
        }

        // Right Box: Live Interactive Preview Chamber
        Rectangle chamberRect = { 650.0f, 145.0f, WINDOW_WIDTH - 710.0f, 495.0f };
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

        m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[TAB] Switch to Jukebox", "[UP / DOWN] Select Skin", "[ENTER / CLICK] Equip Theme" });
    } else {
        // Tab 1: Soundtrack Jukebox
        const auto& catalog = app.GetMusicManager().GetTrackCatalog();
        float cardW = 540.0f;
        float cardH = 56.0f;

        for (size_t i = 0; i < catalog.size(); ++i) {
            Rectangle tRect = { 60.0f, startY + static_cast<float>(i) * (cardH + 6.0f), cardW, cardH };
            bool isHovered = CheckCollisionPointRec(mousePos, tRect);
            bool isSelected = (m_selectedJukeboxIndex == static_cast<int>(i));
            bool isCurrentPlaying = (app.GetMusicManager().GetCurrentTrackId() == catalog[i].id);
            bool isLockedDefault = (m_settings.fixedSoundtrack == static_cast<int>(i + 1));

            if (isHovered && !isSelected) {
                m_selectedJukeboxIndex = static_cast<int>(i);
                app.GetSoundSynth().PlayMenuHover();
            }

            // Click play / lock
            if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (mousePos.x > tRect.x + tRect.width - 130.0f) {
                    // Lock button clicked
                    if (isLockedDefault) {
                        m_settings.fixedSoundtrack = 0;
                        app.GetSoundSynth().PlayMenuBack();
                    } else {
                        m_settings.fixedSoundtrack = static_cast<int>(i + 1);
                        app.GetSoundSynth().PlayLevelUp();
                    }
                    app.GetMusicManager().SetFixedTrackIndex(m_settings.fixedSoundtrack);
                } else {
                    // Play preview
                    app.GetMusicManager().PlayTrack(catalog[i].id, true);
                    app.GetSoundSynth().PlayMenuToggle();
                }
            }

            Color border = isSelected ? Colors::PieceI : (isHovered ? WHITE : Colors::BgPanelBorder);
            Color bg = isSelected ? Fade(Colors::PieceI, 0.15f) : Colors::BgPanel;

            DrawRectangleRounded(tRect, 0.1f, 4, bg);
            DrawRectangleLinesEx(tRect, isSelected ? 2.0f : 1.0f, border);

            // Title & Info
            std::string titleStr = std::to_string(i + 1) + ". " + catalog[i].title;
            DrawText(titleStr.c_str(), static_cast<int>(tRect.x + 14.0f), static_cast<int>(tRect.y + 10.0f), 13, isCurrentPlaying ? Colors::PieceGold : Colors::TextWhite);

            char metaBuf[64];
            snprintf(metaBuf, sizeof(metaBuf), "%s | %.0f BPM | %s", catalog[i].genre.c_str(), catalog[i].baseBpm, catalog[i].artist.c_str());
            DrawText(metaBuf, static_cast<int>(tRect.x + 14.0f), static_cast<int>(tRect.y + 30.0f), 10, Colors::TextDim);

            // Status Badge / Button on right
            Rectangle badgeRect = { tRect.x + tRect.width - 120.0f, tRect.y + 10.0f, 108.0f, 36.0f };
            if (isLockedDefault) {
                DrawRectangleRounded(badgeRect, 0.2f, 4, Fade(Colors::PieceS, 0.25f));
                DrawRectangleLinesEx(badgeRect, 1.2f, Colors::PieceS);
                DrawText("LOCKED RUN", static_cast<int>(badgeRect.x + 12.0f), static_cast<int>(badgeRect.y + 11.0f), 11, Colors::PieceS);
            } else if (isCurrentPlaying) {
                DrawRectangleRounded(badgeRect, 0.2f, 4, Fade(Colors::PieceGold, 0.25f));
                DrawRectangleLinesEx(badgeRect, 1.2f, Colors::PieceGold);
                DrawText("PLAYING ▶", static_cast<int>(badgeRect.x + 16.0f), static_cast<int>(badgeRect.y + 11.0f), 11, Colors::PieceGold);
            } else {
                DrawRectangleRounded(badgeRect, 0.2f, 4, Colors::BgDark);
                DrawRectangleLinesEx(badgeRect, 1.0f, Colors::BgPanelBorder);
                DrawText("PREVIEW ▶", static_cast<int>(badgeRect.x + 16.0f), static_cast<int>(badgeRect.y + 11.0f), 11, Colors::TextDim);
            }
        }

        // Right Box: Cyberpunk Live Audio Visualizer Deck
        Rectangle visRect = { 650.0f, 145.0f, WINDOW_WIDTH - 710.0f, 495.0f };
        DrawRectangleRounded(visRect, 0.04f, 6, Colors::BgPanel);
        DrawRectangleLinesEx(visRect, 1.5f, Colors::BgPanelBorder);

        DrawText("NEURAL AUDIO SYNTHESIZER // JUKEBOX", static_cast<int>(visRect.x + 24.0f), static_cast<int>(visRect.y + 20.0f), 14, Colors::TextAccent);
        DrawLine(static_cast<int>(visRect.x + 24.0f), static_cast<int>(visRect.y + 44.0f), static_cast<int>(visRect.x + visRect.width - 24.0f), static_cast<int>(visRect.y + 44.0f), Colors::BgPanelBorder);

        if (m_selectedJukeboxIndex >= 0 && m_selectedJukeboxIndex < static_cast<int>(catalog.size())) {
            const auto& track = catalog[m_selectedJukeboxIndex];
            DrawText("NOW SELECTED TRACK:", static_cast<int>(visRect.x + 24.0f), static_cast<int>(visRect.y + 65.0f), 12, Colors::TextDim);
            DrawText(track.title.c_str(), static_cast<int>(visRect.x + 24.0f), static_cast<int>(visRect.y + 88.0f), 20, Colors::PieceI);
            
            std::string sub = "GENRE: " + track.genre + "   |   BPM: " + std::to_string(static_cast<int>(track.baseBpm));
            DrawText(sub.c_str(), static_cast<int>(visRect.x + 24.0f), static_cast<int>(visRect.y + 120.0f), 13, Colors::TextWhite);

            // Animated Equalizer Bars
            float barAreaY = visRect.y + 160.0f;
            float barAreaH = 200.0f;
            int numBars = 24;
            float barW = (visRect.width - 48.0f) / static_cast<float>(numBars) - 4.0f;

            for (int b = 0; b < numBars; ++b) {
                float wave = std::sin(m_animTimer * (4.0f + static_cast<float>(b) * 0.4f) + static_cast<float>(b) * 0.6f);
                float barH = (std::abs(wave) * 0.7f + 0.15f) * barAreaH;
                Rectangle bar = {
                    visRect.x + 24.0f + static_cast<float>(b) * (barW + 4.0f),
                    barAreaY + (barAreaH - barH),
                    barW,
                    barH
                };
                Color barCol = (b % 3 == 0) ? Colors::PieceI : ((b % 3 == 1) ? Colors::PieceT : Colors::PieceZ);
                DrawRectangleRounded(bar, 0.3f, 4, Fade(barCol, 0.85f));
            }

            // Lock / Default status description
            std::string statusDesc = (m_settings.fixedSoundtrack == m_selectedJukeboxIndex + 1)
                ? "STATUS: FIXED AS GAME SOUNDTRACK (Plays across all floors)"
                : (m_settings.fixedSoundtrack == 0
                    ? "STATUS: DYNAMIC (Game adapts soundtrack per floor sector)"
                    : "STATUS: ANOTHER TRACK IS CURRENTLY LOCKED");
            DrawText(statusDesc.c_str(), static_cast<int>(visRect.x + 24.0f), static_cast<int>(visRect.y + 390.0f), 12, Colors::PieceGold);

            const char* help = "[ENTER] Set/Unset as Fixed Game Soundtrack   |   [SPACE] Play/Preview";
            DrawText(help, static_cast<int>(visRect.x + 24.0f), static_cast<int>(visRect.y + 440.0f), 11, Colors::TextDim);
        }

        m_menuRenderer.DrawFooterHints({ "[ESC] Return to Main Menu", "[TAB] Switch to Skins", "[UP / DOWN] Select Track", "[SPACE] Play Preview", "[ENTER] Lock as Default" });
    }
}

void TitleState::RenderSettings(GameApp& app) {
    Vector2 mousePos = GetMousePosition();
    m_menuRenderer.DrawHeaderBanner("SYSTEM CONFIGURATION", "SETTINGS", "CONFIG");

    float startY = 88.0f;
    float rowW = WINDOW_WIDTH - 120.0f;
    float rowH = 40.0f;
    float spacing = 6.0f;

    const auto& catalog = app.GetMusicManager().GetTrackCatalog();

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

    // 2. Music Volume Slider
    Rectangle r1 = { 60.0f, startY + (rowH + spacing), rowW, rowH };
    bool h1 = CheckCollisionPointRec(mousePos, r1);
    if (h1 && m_selectedOption != 1) { m_selectedOption = 1; app.GetSoundSynth().PlayMenuHover(); }
    if (h1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float norm = (mousePos.x - (r1.x + 220.0f)) / (r1.width - 340.0f);
        m_settings.musicVolume = std::min(1.0f, std::max(0.0f, norm));
        app.GetMusicManager().SetVolume(m_settings.musicVolume);
    }
    char musicVolBuffer[16];
    snprintf(musicVolBuffer, sizeof(musicVolBuffer), "%d %%", static_cast<int>(m_settings.musicVolume * 100.0f));
    m_menuRenderer.DrawSlider(r1, "MUSIC SOUNDTRACK VOLUME", m_settings.musicVolume, 0.0f, 1.0f, musicVolBuffer, (m_selectedOption == 1));

    // 3. SFX Effects Volume Slider
    Rectangle r2 = { 60.0f, startY + 2.0f * (rowH + spacing), rowW, rowH };
    bool h2 = CheckCollisionPointRec(mousePos, r2);
    if (h2 && m_selectedOption != 2) { m_selectedOption = 2; app.GetSoundSynth().PlayMenuHover(); }
    if (h2 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float norm = (mousePos.x - (r2.x + 220.0f)) / (r2.width - 340.0f);
        m_settings.sfxVolume = std::min(1.0f, std::max(0.0f, norm));
    }
    char sfxVolBuffer[16];
    snprintf(sfxVolBuffer, sizeof(sfxVolBuffer), "%d %%", static_cast<int>(m_settings.sfxVolume * 100.0f));
    m_menuRenderer.DrawSlider(r2, "SOUND FX VOLUME", m_settings.sfxVolume, 0.0f, 1.0f, sfxVolBuffer, (m_selectedOption == 2));

    // 4. Fixed Gameplay Soundtrack Selection
    Rectangle r3 = { 60.0f, startY + 3.0f * (rowH + spacing), rowW, rowH };
    bool h3 = CheckCollisionPointRec(mousePos, r3);
    if (h3 && m_selectedOption != 3) { m_selectedOption = 3; app.GetSoundSynth().PlayMenuHover(); }
    if (h3 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.fixedSoundtrack = (m_settings.fixedSoundtrack + 1) % 9;
        app.GetMusicManager().SetFixedTrackIndex(m_settings.fixedSoundtrack);
        if (m_settings.fixedSoundtrack > 0 && m_settings.fixedSoundtrack <= static_cast<int>(catalog.size())) {
            app.GetMusicManager().PlayTrack(catalog[m_settings.fixedSoundtrack - 1].id, true);
        } else {
            app.GetMusicManager().PlayTrack(TrackId::MenuTheme, true);
        }
        app.GetSoundSynth().PlayMenuToggle();
    }
    std::string trackLabel;
    if (m_settings.fixedSoundtrack == 0) {
        trackLabel = "DYNAMIC (CHANGES BY FLOOR SECTOR)";
    } else {
        trackLabel = "FIXED: " + std::to_string(m_settings.fixedSoundtrack) + ". " + catalog[m_settings.fixedSoundtrack - 1].title;
    }
    m_menuRenderer.DrawSlider(r3, "GAMEPLAY SOUNDTRACK MODE", static_cast<float>(m_settings.fixedSoundtrack), 0.0f, 8.0f, trackLabel.c_str(), (m_selectedOption == 3));

    // 5. Screen Shake Intensity
    Rectangle r4 = { 60.0f, startY + 4.0f * (rowH + spacing), rowW, rowH };
    bool h4 = CheckCollisionPointRec(mousePos, r4);
    if (h4 && m_selectedOption != 4) { m_selectedOption = 4; app.GetSoundSynth().PlayMenuHover(); }
    if (h4 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.screenShakeLevel = (m_settings.screenShakeLevel + 1) % 4;
        app.GetSoundSynth().PlayMenuToggle();
    }
    const char* shakeLabels[4] = { "DISABLED (0x)", "SUBTLE (0.5x)", "STANDARD (1.0x)", "INTENSE (1.5x)" };
    m_menuRenderer.DrawSlider(r4, "IMPACT SCREEN SHAKE", static_cast<float>(m_settings.screenShakeLevel), 0.0f, 3.0f, shakeLabels[m_settings.screenShakeLevel], (m_selectedOption == 4));

    // 6. CRT Scanlines & Bloom Toggle
    Rectangle r5 = { 60.0f, startY + 5.0f * (rowH + spacing), rowW, rowH };
    bool h5 = CheckCollisionPointRec(mousePos, r5);
    if (h5 && m_selectedOption != 5) { m_selectedOption = 5; app.GetSoundSynth().PlayMenuHover(); }
    if (h5 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.crtScanlines = !m_settings.crtScanlines;
        app.GetSoundSynth().PlayMenuToggle();
    }
    m_menuRenderer.DrawToggle(r5, "POST-PROCESS CRT SCANLINES & BLOOM", m_settings.crtScanlines, (m_selectedOption == 5));

    // 7. Soft-Body Wobble Intensity
    Rectangle r6 = { 60.0f, startY + 6.0f * (rowH + spacing), rowW, rowH };
    bool h6 = CheckCollisionPointRec(mousePos, r6);
    if (h6 && m_selectedOption != 6) { m_selectedOption = 6; app.GetSoundSynth().PlayMenuHover(); }
    if (h6 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.softBodyWobble = (m_settings.softBodyWobble + 1) % 3;
        app.GetSoundSynth().PlayMenuToggle();
    }
    const char* wobbleLabels[3] = { "RELAXED (MILD SQUISH)", "STANDARD (SPRING LATTICE)", "ULTRA JELLY (MAX ELASTICITY)" };
    m_menuRenderer.DrawSlider(r6, "SOFT-BODY DEFORMATION DEGREE", static_cast<float>(m_settings.softBodyWobble), 0.0f, 2.0f, wobbleLabels[m_settings.softBodyWobble], (m_selectedOption == 6));

    // 8. Fast DAS / ARR Handling
    Rectangle r7 = { 60.0f, startY + 7.0f * (rowH + spacing), rowW, rowH };
    bool h7 = CheckCollisionPointRec(mousePos, r7);
    if (h7 && m_selectedOption != 7) { m_selectedOption = 7; app.GetSoundSynth().PlayMenuHover(); }
    if (h7 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        m_settings.fastDAS = !m_settings.fastDAS;
        app.GetSoundSynth().PlayMenuToggle();
    }
    m_menuRenderer.DrawToggle(r7, "TOURNAMENT DAS & ARR RESPONSE (HIGH REPEAT)", m_settings.fastDAS, (m_selectedOption == 7));

    // Keybindings Quick Overview Box
    Rectangle kbBox = { 60.0f, startY + 8.0f * (rowH + spacing) + 8.0f, rowW, 110.0f };
    DrawRectangleRounded(kbBox, 0.08f, 4, Colors::BgPanel);
    DrawRectangleLinesEx(kbBox, 1.0f, Colors::BgPanelBorder);
    DrawText("DEFAULT INPUT BINDINGS:", static_cast<int>(kbBox.x + 16.0f), static_cast<int>(kbBox.y + 14.0f), 12, Colors::TextAccent);
    DrawText("[LEFT / RIGHT / DOWN] Move & Soft Drop   |   [UP / X] Rotate CW   |   [Z] Rotate CCW", static_cast<int>(kbBox.x + 16.0f), static_cast<int>(kbBox.y + 36.0f), 12, Colors::TextWhite);
    DrawText("[SPACE] Hard Drop   |   [C / SHIFT] Hold Piece   |   [1 / 2] Trigger Active Relic Cards", static_cast<int>(kbBox.x + 16.0f), static_cast<int>(kbBox.y + 58.0f), 12, Colors::TextWhite);
    DrawText("[P / ESC] Pause Game   |   [F1] Toggle Verlet Springs Debug   |   [F2] Card Draft Test", static_cast<int>(kbBox.x + 16.0f), static_cast<int>(kbBox.y + 80.0f), 12, Colors::TextDim);

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        std::string eqTheme = "THEME_NEON";
        for (const auto& t : m_customThemes) if (t.isEquipped) eqTheme = t.id;
        app.GetSaveManager().SaveProfile(m_profile, m_settings, m_shopItems, eqTheme);
    }

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
        app.RequestExit();
        return;
    }

    m_menuRenderer.DrawNeonButton(btnCancel, "RESUME [ESC]", nullptr, (m_selectedOption == 0), hoverCancel, Colors::TextGreen);
    m_menuRenderer.DrawNeonButton(btnConfirm, "EXIT TO OS", nullptr, (m_selectedOption == 1), hoverConfirm, Colors::PieceBomb);
}

} // namespace TetroShift
