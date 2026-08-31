#include "PlayState.hpp"
#include "TitleState.hpp"
#include "CardDraftState.hpp"
#include "GameOverState.hpp"
#include "core/GameApp.hpp"
#include "core/Constants.hpp"
#include <algorithm>

namespace TetroShift {

PlayState::PlayState(int activeSlot, std::optional<SavedRunState> restoredRun)
    : m_activeSlot(activeSlot), m_restoredRun(std::move(restoredRun)) {
    m_grid = std::make_unique<OrthogonalGrid>(GRID_WIDTH, GRID_HEIGHT, GRID_BUFFER_HEIGHT);
}

SavedRunState PlayState::ExportCurrentRunState() const {
    SavedRunState state;
    state.slotId = m_activeSlot;
    state.state = SaveSlotState::ActiveRun;
    state.runMode = "ROGUELIKE RUN // SECTOR 0" + std::to_string(m_runManager.GetFloor());
    state.floor = m_runManager.GetFloor();
    state.score = m_runManager.GetScore();
    state.linesTotal = m_runManager.GetLinesTotal();
    state.linesThisFloor = m_runManager.GetLinesThisFloor();
    state.floorLineTarget = m_runManager.GetFloorLineTarget();
    state.coins = m_runManager.GetInventory().GetCoins();
    state.rerollTokens = m_runManager.GetInventory().GetRerolls();
    state.scoreMultiplier = m_runManager.GetScoreMultiplier();
    state.speedMultiplier = m_runManager.GetSpeedMultiplier();
    state.globalElasticity = m_runManager.GetGlobalElasticity();
    auto holdOpt = m_spawner.GetHoldPiece();
    state.holdPiece = holdOpt.has_value() ? holdOpt.value() : TetrominoType::None;
    state.canHold = m_spawner.CanHold();
    state.rngState = 1337;

    for (const auto& card : m_runManager.GetInventory().GetPassives()) {
        state.cardIds.push_back(card.id);
    }
    for (const auto& card : m_runManager.GetInventory().GetActives()) {
        state.cardIds.push_back(card.id);
    }

    state.gridCells = m_grid->Serialize();
    return state;
}

void PlayState::OnEnter(GameApp& app) {
    m_grid->Clear();
    m_spawner.Reset(1337);
    m_runManager.Reset();
    m_particles.Reset();
    m_screenEffects.Reset();

    m_dasTimer = 0.0f;
    m_arrTimer = 0.0f;
    m_heldDirection = 0;
    m_showDebugPhysics = false;
    m_isPaused = false;

    // Check if we are restoring a suspended saved run!
    if (m_restoredRun.has_value()) {
        const auto& saved = *m_restoredRun;
        m_runManager.SetScore(saved.score);
        m_runManager.SetFloor(saved.floor);
        m_runManager.SetLinesTotal(saved.linesTotal);
        m_runManager.SetLinesThisFloor(saved.linesThisFloor);
        m_runManager.SetFloorLineTarget(saved.floorLineTarget);
        m_runManager.SetScoreModifier(saved.scoreMultiplier);
        m_runManager.SetBaseFallSpeedMultiplier(saved.speedMultiplier);
        m_runManager.SetGlobalElasticity(saved.globalElasticity);

        m_runManager.GetInventory().SetCoins(saved.coins);
        m_runManager.GetInventory().SetRerolls(saved.rerollTokens);

        // Restore grid
        m_grid->Deserialize(saved.gridCells);

        // Restore Spawner
        m_spawner.SetHoldPiece(saved.holdPiece);
        m_spawner.SetCanHold(saved.canHold);
    }

    // Subscribe to card acquisition
    app.GetEventBus().Subscribe<EventCardAcquired>([this, &app](const EventCardAcquired& e) {
        const Card* card = app.GetCardDatabase().FindCardById(e.cardId);
        if (card) {
            CardContext ctx{ &m_runManager, &m_activePiece, m_grid.get(), &m_spawner, &app.GetEventBus() };
            m_runManager.GetInventory().AddCard(*card, ctx);
            m_runManager.AdvanceFloor();
            m_hazardManager.SetFloor(m_runManager.GetFloor());

            // Check if track changes on floor transition (only if dynamic mode)
            if (app.GetMusicManager().GetFixedTrackIndex() == 0) {
                int floor = m_runManager.GetFloor();
                TrackId theme = TrackId::EarlyFloorTheme;
                if (floor >= 10) theme = TrackId::BossFloorTheme;
                else if (floor >= 7) theme = TrackId::HighFloorTheme;
                else if (floor >= 4) theme = TrackId::MidFloorTheme;
                app.GetMusicManager().PlayTrack(theme, true);
            }
        }
    });

    // If restoring saved cards, add them after subscription
    if (m_restoredRun.has_value()) {
        for (const auto& cardId : m_restoredRun->cardIds) {
            const Card* card = app.GetCardDatabase().FindCardById(cardId);
            if (card) {
                CardContext ctx{ &m_runManager, &m_activePiece, m_grid.get(), &m_spawner, &app.GetEventBus() };
                m_runManager.GetInventory().AddCard(*card, ctx);
            }
        }
    }

    // Start floor soundtrack (respecting fixed track preference)
    if (app.GetMusicManager().GetFixedTrackIndex() > 0) {
        app.GetMusicManager().PlayTrack(app.GetMusicManager().GetFixedTrackId(), true);
    } else {
        int initialFloor = m_runManager.GetFloor();
        TrackId initialTheme = TrackId::EarlyFloorTheme;
        if (initialFloor >= 10) initialTheme = TrackId::BossFloorTheme;
        else if (initialFloor >= 7) initialTheme = TrackId::HighFloorTheme;
        else if (initialFloor >= 4) initialTheme = TrackId::MidFloorTheme;
        app.GetMusicManager().PlayTrack(initialTheme, true);
    }

    m_hazardManager.SetFloor(m_runManager.GetFloor());
    SpawnNextPiece(app);
}

void PlayState::OnExit(GameApp& /*app*/) {}

void PlayState::SpawnNextPiece(GameApp& app) {
    TetrominoType nextType = m_spawner.PopNext();
    CellType minoType = m_spawner.DetermineSpawnCellType();
    float elasticity = m_runManager.GetGlobalElasticity();

    // Standard spawn position for 10-wide grid
    GridCoord startPos = { 3, 0 };
    if (nextType == TetrominoType::O) {
        startPos = { 4, 0 };
    }

    m_activePiece.Spawn(nextType, startPos, minoType, elasticity);

    CardContext ctx{ &m_runManager, &m_activePiece, m_grid.get(), &m_spawner, &app.GetEventBus() };
    m_runManager.RegisterPieceSpawn(ctx);

    // Check if initial position collides (Top Out / Game Over)
    const auto coords = m_activePiece.GetMinoGridCoords();
    for (const auto& c : coords) {
        if (m_grid->IsCellOccupied(c)) {
            // Check if Deflector Shield saves the player!
            if (m_runManager.GetInventory().ConsumeShield()) {
                app.GetSoundSynth().PlayLevelUp();
                app.GetSoundSynth().PlayLineClear(4);
                m_screenEffects.TriggerFlash(GOLD, 0.6f);
                m_screenEffects.AddTrauma(0.5f);
                m_grid->VaporizeTopRows(6);
                m_particles.AddPopup("🛡 DEFLECTOR SHIELD SAVED YOU!", { PLAYFIELD_X + 160.0f, PLAYFIELD_Y + 120.0f }, GOLD, 1.8f);
                return;
            }

            // Game Over!
            app.GetSoundSynth().PlayGameOver();
            m_screenEffects.TriggerFlash(RED, 0.6f);

            // Persist run results to career profile & high scores
            app.GetSaveManager().AwardRunResults(
                m_runManager.GetScore(),
                m_runManager.GetFloor(),
                m_runManager.GetLinesTotal(),
                false
            );
            app.GetSaveManager().DeleteRunSlot(m_activeSlot);

            HighScoreEntry entry;
            entry.pilotName = app.GetSaveManager().GetProfile().pilotCallsign;
            entry.score = m_runManager.GetScore();
            entry.floorReached = m_runManager.GetFloor();
            entry.linesCleared = m_runManager.GetLinesTotal();
            app.GetSaveManager().AddHighScoreEntry(entry);

            app.GetStateManager().SetState(app, std::make_unique<GameOverState>(
                m_runManager.GetScore(),
                m_runManager.GetFloor(),
                m_runManager.GetLinesTotal()
            ));
            return;
        }
    }
}

void PlayState::HandlePieceLock(GameApp& app) {
    app.GetSoundSynth().PlayLock();

    // 1. Detect T-Spin status before committing cells to grid
    TSpinType tspin = m_activePiece.CheckTSpin(*m_grid);

    const auto coords = m_activePiece.GetMinoGridCoords();
    const Color pieceColor = GetTetrominoColor(m_activePiece.GetType());
    const CellType minoType = m_activePiece.GetMinoType();

    // Commit cells to the grid
    for (const auto& c : coords) {
        Cell cell;
        cell.type = minoType;
        cell.color = (minoType == CellType::Gold) ? Colors::PieceGold :
                     (minoType == CellType::Bomb) ? Colors::PieceBomb :
                     (minoType == CellType::Jelly) ? Colors::PieceJelly :
                     (minoType == CellType::Sand) ? Colors::PieceSand : pieceColor;
        cell.elasticity = m_runManager.GetGlobalElasticity();
        cell.isLocked = true;
        cell.flashTimer = 0.4f;
        m_grid->SetCell(c, cell);
    }

    // Unlock hold capability
    m_spawner.UnlockHold();

    // Context for card hooks
    CardContext ctx{ &m_runManager, &m_activePiece, m_grid.get(), &m_spawner, &app.GetEventBus() };
    m_runManager.RegisterPieceLock(ctx);

    // Check line clears
    LineClearResult clearResult = m_grid->CheckAndClearLines();

    if (clearResult.linesCount > 0 || tspin != TSpinType::None) {
        bool isDifficult = (clearResult.isTetris || (tspin != TSpinType::None && clearResult.linesCount > 0));
        bool isB2B = (isDifficult && m_runManager.GetB2BStreak() > 0);

        // Audio feedback
        if (tspin != TSpinType::None) {
            app.GetSoundSynth().PlayTSpin();
        } else if (isB2B) {
            app.GetSoundSynth().PlayBackToBack();
        } else if (!clearResult.bombExplosions.empty()) {
            app.GetSoundSynth().PlayBombExplosion();
        } else {
            app.GetSoundSynth().PlayLineClear(clearResult.linesCount);
        }

        if (m_runManager.GetCombo() > 0) {
            app.GetSoundSynth().PlayCombo(m_runManager.GetCombo());
        }

        // Screen shake trauma
        float trauma = (clearResult.linesCount >= 4 || tspin != TSpinType::None) ? 0.60f : (0.15f * static_cast<float>(clearResult.linesCount));
        m_screenEffects.AddTrauma(trauma);
        if (clearResult.linesCount >= 4 || tspin != TSpinType::None) {
            m_screenEffects.TriggerFlash((tspin != TSpinType::None) ? Colors::PieceT : Colors::TextAccent, 0.45f);
        }

        // Emit particles along cleared rows
        const float gridOriginX = PLAYFIELD_X;
        const float gridWidthPx = static_cast<float>(m_grid->GetWidth()) * CELL_SIZE;
        for (int row : clearResult.rowsCleared) {
            float rowWorldY = PLAYFIELD_Y + static_cast<float>(row) * CELL_SIZE;
            m_particles.EmitLineClear(rowWorldY, gridOriginX, gridOriginX + gridWidthPx, pieceColor, 35);
        }

        // Bomb explosions
        for (const auto& b : clearResult.bombExplosions) {
            Vector2 bWorld = m_grid->CoordToWorld(b, { PLAYFIELD_X, PLAYFIELD_Y }, CELL_SIZE);
            bWorld.x += CELL_SIZE * 0.5f;
            bWorld.y += CELL_SIZE * 0.5f;
            m_particles.EmitBombBlast(bWorld, RED, 50);
            m_screenEffects.AddTrauma(0.35f);
        }

        // Floating score popup
        std::string popupText = "";
        Color popupCol = Colors::TextAccent;

        if (tspin == TSpinType::Standard) {
            popupCol = Colors::PieceT;
            if (clearResult.linesCount == 0) popupText = "T-SPIN!";
            else if (clearResult.linesCount == 1) popupText = "T-SPIN SINGLE!";
            else if (clearResult.linesCount == 2) popupText = "T-SPIN DOUBLE!";
            else if (clearResult.linesCount == 3) popupText = "T-SPIN TRIPLE!";
        } else if (tspin == TSpinType::Mini) {
            popupCol = Colors::PieceT;
            if (clearResult.linesCount == 0) popupText = "T-SPIN MINI";
            else if (clearResult.linesCount == 1) popupText = "T-SPIN MINI SINGLE";
            else if (clearResult.linesCount == 2) popupText = "T-SPIN MINI DOUBLE";
        } else if (clearResult.linesCount >= 4) {
            popupCol = GOLD;
            popupText = "+800 TETRIS!";
        } else if (clearResult.linesCount > 0) {
            popupText = "+" + std::to_string(clearResult.linesCount * 100);
        }

        if (isB2B) {
            popupText = "B2B " + popupText;
            popupCol = Colors::PieceGold;
        }

        if (clearResult.coinsGenerated > 0) {
            popupText += " (+$" + std::to_string(clearResult.coinsGenerated) + ")";
        }

        Vector2 centerPlayfield = { PLAYFIELD_X + gridWidthPx * 0.5f, PLAYFIELD_Y + CELL_SIZE * 6.0f };
        m_particles.AddPopup(popupText, centerPlayfield, popupCol, (clearResult.linesCount >= 4 || tspin != TSpinType::None) ? 1.5f : 1.0f);

        // Register with run manager
        m_runManager.RegisterLineClear(clearResult.linesCount, clearResult.linesCount >= 4, ctx, tspin);

        // Check if floor cleared and draft pending
        if (m_runManager.IsDraftPending()) {
            m_runManager.ConsumeDraftFlag();
            app.GetSoundSynth().PlayLevelUp();
            m_screenEffects.TriggerFlash(GOLD, 0.5f);
            app.GetStateManager().PushOverlay(app, std::make_unique<CardDraftState>(m_runManager.GetFloor()));
            return;
        }
    } else {
        m_runManager.RegisterLineClear(0, false, ctx, TSpinType::None);
    }

    // Spawn next piece
    SpawnNextPiece(app);
}

void PlayState::HandleMovementInput(GameApp& app, float dt) {
    // 1. DAS / ARR Horizontal Movement
    int currentDir = 0;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) currentDir = -1;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) currentDir = +1;

    // Apply Glitch Matrix hazard command inversion
    if (m_hazardManager.AreControlsInverted()) {
        currentDir = -currentDir;
    }

    if (currentDir != 0) {
        if (m_heldDirection != currentDir) {
            // First tap
            m_heldDirection = currentDir;
            m_dasTimer = 0.0f;
            m_arrTimer = 0.0f;
            if (m_activePiece.TryMove(GridCoord{ currentDir, 0 }, *m_grid, &app.GetEventBus())) {
                app.GetSoundSynth().PlayMove();
            }
        } else {
            // Key is held
            m_dasTimer += dt;
            if (m_dasTimer >= DAS_DELAY) {
                m_arrTimer += dt;
                while (m_arrTimer >= ARR_INTERVAL) {
                    m_arrTimer -= ARR_INTERVAL;
                    m_activePiece.TryMove(GridCoord{ currentDir, 0 }, *m_grid, &app.GetEventBus());
                }
            }
        }
    } else {
        m_heldDirection = 0;
        m_dasTimer = 0.0f;
        m_arrTimer = 0.0f;
    }

    // 2. Rotations
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_X) || IsKeyPressed(KEY_W)) {
        if (m_activePiece.TryRotate(+1, *m_grid, &app.GetEventBus())) {
            app.GetSoundSynth().PlayRotate();
        }
    }
    if (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_LEFT_CONTROL)) {
        if (m_activePiece.TryRotate(-1, *m_grid, &app.GetEventBus())) {
            app.GetSoundSynth().PlayRotate();
        }
    }

    // 3. Hard Drop
    if (IsKeyPressed(KEY_SPACE)) {
        int dropped = m_activePiece.HardDrop(*m_grid, &app.GetEventBus());
        if (dropped > 0) {
            app.GetSoundSynth().PlayDrop();
            m_screenEffects.AddTrauma(0.18f);

            // Dust particles at landing
            const auto coords = m_activePiece.GetMinoGridCoords();
            for (const auto& c : coords) {
                Vector2 world = m_grid->CoordToWorld(c, { PLAYFIELD_X, PLAYFIELD_Y }, CELL_SIZE);
                m_particles.EmitHardDropDust({ world.x + CELL_SIZE * 0.5f, world.y + CELL_SIZE }, GetTetrominoColor(m_activePiece.GetType()), 8);
            }
        }
        HandlePieceLock(app);
        return;
    }

    // 4. Hold Piece
    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_LEFT_SHIFT)) {
        std::optional<TetrominoType> swapped;
        if (m_spawner.TryHold(m_activePiece.GetType(), swapped)) {
            app.GetSoundSynth().PlayMove();
            if (swapped.has_value()) {
                m_activePiece.Spawn(swapped.value(), { 3, 0 }, m_spawner.DetermineSpawnCellType(), m_runManager.GetGlobalElasticity());
            } else {
                SpawnNextPiece(app);
            }
        }
    }

    // 5. Active Abilities [Keys 1 & 2]
    if (IsKeyPressed(KEY_ONE)) {
        CardContext ctx{ &m_runManager, &m_activePiece, m_grid.get(), &m_spawner, &app.GetEventBus() };
        if (m_runManager.GetInventory().TryUseActiveAbility(0, ctx)) {
            app.GetSoundSynth().PlayCardSelect();
            m_screenEffects.AddTrauma(0.2f);
            m_particles.AddPopup("ABILITY ACTIVATED!", { PLAYFIELD_X + 160.0f, PLAYFIELD_Y + 100.0f }, Colors::TextAccent, 1.2f);
        }
    }
    if (IsKeyPressed(KEY_TWO)) {
        CardContext ctx{ &m_runManager, &m_activePiece, m_grid.get(), &m_spawner, &app.GetEventBus() };
        if (m_runManager.GetInventory().TryUseActiveAbility(1, ctx)) {
            app.GetSoundSynth().PlayCardSelect();
            m_screenEffects.AddTrauma(0.2f);
            m_particles.AddPopup("ABILITY ACTIVATED!", { PLAYFIELD_X + 160.0f, PLAYFIELD_Y + 100.0f }, Colors::TextAccent, 1.2f);
        }
    }

    // 6. Fast Debug Keys
    if (IsKeyPressed(KEY_F1)) {
        m_showDebugPhysics = !m_showDebugPhysics;
    }
    if (IsKeyPressed(KEY_F2)) {
        // Force Draft
        app.GetSoundSynth().PlayLevelUp();
        app.GetStateManager().PushOverlay(app, std::make_unique<CardDraftState>(m_runManager.GetFloor()));
    }
    if (IsKeyPressed(KEY_F3)) {
        TriggerInstantLineClear(app);
    }
    if (IsKeyPressed(KEY_F4)) {
        m_screenEffects.ToggleScanlines();
    }
    if (IsKeyPressed(KEY_F5)) {
        OnEnter(app);
    }
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        m_isPaused = !m_isPaused;
    }
}

void PlayState::TriggerInstantLineClear(GameApp& app) {
    // Fill bottom row and trigger clear
    for (int x = 0; x < m_grid->GetWidth(); ++x) {
        Cell c;
        c.type = CellType::Solid;
        c.color = Colors::PieceI;
        m_grid->SetCell({ x, m_grid->GetHeight() - 1 }, c);
    }
    LineClearResult res = m_grid->CheckAndClearLines();
    if (res.linesCount > 0) {
        app.GetSoundSynth().PlayLineClear(res.linesCount);
        m_screenEffects.AddTrauma(0.3f);
        CardContext ctx{ &m_runManager, &m_activePiece, m_grid.get(), &m_spawner, &app.GetEventBus() };
        m_runManager.RegisterLineClear(res.linesCount, false, ctx);
    }
}

void PlayState::HandleInput(GameApp& app) {
    // ESC or P toggles Pause
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
        m_isPaused = !m_isPaused;
        if (m_isPaused) {
            m_pauseSelectedOption = 0;
            app.GetSoundSynth().PlayMenuToggle();
        } else {
            app.GetSoundSynth().PlayMenuBack();
        }
        return;
    }

    if (m_isPaused) {
        const int numPauseOptions = 5;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            m_pauseSelectedOption = (m_pauseSelectedOption - 1 + numPauseOptions) % numPauseOptions;
            app.GetSoundSynth().PlayMenuHover();
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            m_pauseSelectedOption = (m_pauseSelectedOption + 1) % numPauseOptions;
            app.GetSoundSynth().PlayMenuHover();
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            switch (m_pauseSelectedOption) {
                case 0: // Resume
                    m_isPaused = false;
                    app.GetSoundSynth().PlayCardSelect();
                    break;
                case 1: // Save & Suspend
                    app.GetSaveManager().SaveRunSlot(m_activeSlot, ExportCurrentRunState());
                    app.GetSoundSynth().PlayMenuBack();
                    app.GetStateManager().SetState(app, std::make_unique<TitleState>());
                    return;
                case 2: // Restart Run
                    m_isPaused = false;
                    app.GetSoundSynth().PlayCardSelect();
                    m_restoredRun = std::nullopt;
                    OnEnter(app);
                    break;
                case 3: // Toggle Sound Mute
                    app.GetSoundSynth().SetMuted(!app.GetSoundSynth().IsMuted());
                    app.GetSoundSynth().PlayMenuToggle();
                    break;
                case 4: // Return to Main Menu (Abandon)
                    app.GetSoundSynth().PlayMenuBack();
                    app.GetStateManager().SetState(app, std::make_unique<TitleState>());
                    return;
            }
        }
        return;
    }

    HandleMovementInput(app, GetFrameTime());
}

void PlayState::Update(GameApp& app, float dt) {
    if (m_isPaused || app.GetStateManager().HasOverlay()) return;

    m_hazardManager.Update(dt, *m_grid, m_activePiece, m_screenEffects, m_particles, app.GetSoundSynth());

    m_grid->Update(dt);
    m_particles.Update(dt);
    m_screenEffects.Update(dt);

    // Calculate effective fall interval (accelerated on Soft Drop and Hazard Gravity)
    float speedMult = m_runManager.GetSpeedMultiplier() * m_hazardManager.GetGravitySpeedMultiplier();
    if (m_runManager.GetInventory().HasCryoBuff()) speedMult *= 0.8f;
    float fallInterval = m_runManager.GetFallInterval() / std::max(0.1f, speedMult);
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        fallInterval /= SOFT_DROP_FACTOR;
    }

    m_activePiece.Update(dt, fallInterval, *m_grid, &app.GetEventBus());

    // Check if piece locked via lock delay
    if (m_activePiece.IsLocked()) {
        HandlePieceLock(app);
    }

    // Dynamic music urgency modulation based on highest occupied row
    int highestOccupiedRow = GRID_HEIGHT;
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        bool rowOccupied = false;
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (m_grid->IsCellOccupied({x, y})) {
                rowOccupied = true;
                break;
            }
        }
        if (rowOccupied) {
            highestOccupiedRow = y;
            break;
        }
    }
    float urgency = 0.0f;
    if (highestOccupiedRow < 10) {
        urgency = static_cast<float>(10 - highestOccupiedRow) / 10.0f;
    }
    app.GetMusicManager().SetUrgencyFactor(urgency);
}

void PlayState::RenderPauseMenu(GameApp& app) {
    Vector2 mousePos = GetMousePosition();

    // Dark backdrop
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.75f));

    // Pause Modal Card
    Rectangle modal = { WINDOW_WIDTH * 0.5f - 230.0f, WINDOW_HEIGHT * 0.5f - 240.0f, 460.0f, 480.0f };
    DrawRectangleRounded(modal, 0.08f, 6, Colors::BgDark);
    DrawRectangleLinesEx(modal, 2.0f, Colors::PieceI);

    // Header strip
    Rectangle headerRect = { modal.x + 2.0f, modal.y + 2.0f, modal.width - 4.0f, 44.0f };
    DrawRectangleRounded(headerRect, 0.08f, 4, Fade(Colors::PieceI, 0.2f));
    DrawText("TACTICAL PAUSE // SUSPENDED", static_cast<int>(headerRect.x + 20.0f), static_cast<int>(headerRect.y + 14.0f), 16, Colors::TextWhite);

    // Mini Stats Capsule
    Rectangle statsRect = { modal.x + 20.0f, modal.y + 54.0f, modal.width - 40.0f, 38.0f };
    DrawRectangleRounded(statsRect, 0.15f, 4, Colors::BgPanel);
    DrawRectangleLinesEx(statsRect, 1.0f, Colors::BgPanelBorder);

    std::string statsStr = "SLOT #" + std::to_string(m_activeSlot) +
                           "  |  FL: " + std::to_string(m_runManager.GetFloor()) +
                           "  |  SCORE: " + std::to_string(m_runManager.GetScore()) +
                           "  |  LINES: " + std::to_string(m_runManager.GetLinesTotal()) +
                           "  |  $" + std::to_string(m_runManager.GetInventory().GetCoins());
    DrawText(statsStr.c_str(), static_cast<int>(statsRect.x + 14.0f), static_cast<int>(statsRect.y + 12.0f), 12, Colors::TextAccent);

    // 5 Pause Menu Buttons
    struct PauseBtn {
        const char* label;
        const char* badge;
        Color accent;
    };

    std::string audioBadge = app.GetSoundSynth().IsMuted() ? "MUTED" : "ACTIVE";
    const PauseBtn btns[] = {
        { "RESUME MISSION", "[ESC/P]", Colors::TextGreen },
        { "SAVE & SUSPEND RUN", "SAVE & MENU", Colors::PieceI },
        { "RESTART RUN", "RETRY", Colors::PieceGold },
        { app.GetSoundSynth().IsMuted() ? "AUDIO: UNMUTE" : "AUDIO: MUTE", audioBadge.c_str(), Colors::PieceT },
        { "ABANDON RUN & MENU", "QUIT", Colors::PieceBomb }
    };

    float startY = modal.y + 104.0f;
    float btnH = 46.0f;
    float spacing = 10.0f;

    for (int i = 0; i < 5; ++i) {
        Rectangle bRect = { modal.x + 20.0f, startY + static_cast<float>(i) * (btnH + spacing), modal.width - 40.0f, btnH };
        bool isHovered = CheckCollisionPointRec(mousePos, bRect);
        bool isSelected = (m_pauseSelectedOption == i);

        if (isHovered && !isSelected) {
            m_pauseSelectedOption = i;
            app.GetSoundSynth().PlayMenuHover();
        }

        if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            switch (i) {
                case 0:
                    m_isPaused = false;
                    app.GetSoundSynth().PlayCardSelect();
                    break;
                case 1:
                    app.GetSaveManager().SaveRunSlot(m_activeSlot, ExportCurrentRunState());
                    app.GetSoundSynth().PlayMenuBack();
                    app.GetStateManager().SetState(app, std::make_unique<TitleState>());
                    return;
                case 2:
                    m_isPaused = false;
                    app.GetSoundSynth().PlayCardSelect();
                    m_restoredRun = std::nullopt;
                    OnEnter(app);
                    break;
                case 3:
                    app.GetSoundSynth().SetMuted(!app.GetSoundSynth().IsMuted());
                    app.GetSoundSynth().PlayMenuToggle();
                    break;
                case 4:
                    app.GetSoundSynth().PlayMenuBack();
                    app.GetStateManager().SetState(app, std::make_unique<TitleState>());
                    return;
            }
        }

        m_menuRenderer.DrawNeonButton(bRect, btns[i].label, btns[i].badge, isSelected, isHovered, btns[i].accent);
    }

    // Modal Footer Hint
    const char* hint = "[ESC / P] Resume  *  [ARROWS / MOUSE] Navigate  *  [ENTER] Select";
    int hw = MeasureText(hint, 10);
    DrawText(hint, static_cast<int>(modal.x + (modal.width - hw) * 0.5f), static_cast<int>(modal.y + modal.height - 20.0f), 10, Colors::TextDim);
}

void PlayState::Render(GameApp& app) {
    ClearBackground(Colors::BgDark);

    m_renderer.RenderGameHUD(
        *m_grid,
        m_activePiece,
        m_spawner,
        m_runManager,
        m_particles,
        m_screenEffects,
        m_showDebugPhysics,
        &m_hazardManager
    );

    // HUD Now Playing Banner
    if (app.GetMusicManager().IsNowPlayingVisible()) {
        m_renderer.DrawNowPlayingBanner(
            app.GetMusicManager().GetNowPlayingTitle().c_str(),
            app.GetMusicManager().GetNowPlayingGenre().c_str(),
            app.GetMusicManager().GetNowPlayingAlpha()
        );
    }

    if (m_isPaused) {
        RenderPauseMenu(app);
    }
}

} // namespace TetroShift
