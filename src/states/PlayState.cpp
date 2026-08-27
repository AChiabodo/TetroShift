#include "PlayState.hpp"
#include "CardDraftState.hpp"
#include "GameOverState.hpp"
#include "core/GameApp.hpp"
#include "core/Constants.hpp"
#include <algorithm>

namespace TetroShift {

PlayState::PlayState() {
    m_grid = std::make_unique<OrthogonalGrid>(GRID_WIDTH, GRID_HEIGHT, GRID_BUFFER_HEIGHT);
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

    // Subscribe to card acquisition
    app.GetEventBus().Subscribe<EventCardAcquired>([this, &app](const EventCardAcquired& e) {
        const Card* card = app.GetCardDatabase().FindCardById(e.cardId);
        if (card) {
            CardContext ctx{ &m_runManager, &m_activePiece, m_grid.get(), &m_spawner, &app.GetEventBus() };
            m_runManager.GetInventory().AddCard(*card, ctx);
            m_runManager.AdvanceFloor();
        }
    });

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
            // Game Over!
            app.GetSoundSynth().PlayGameOver();
            m_screenEffects.TriggerFlash(RED, 0.6f);
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

    const auto coords = m_activePiece.GetMinoGridCoords();
    const Color pieceColor = GetTetrominoColor(m_activePiece.GetType());
    const CellType minoType = m_activePiece.GetMinoType();

    // Commit cells to the grid
    for (const auto& c : coords) {
        Cell cell;
        cell.type = minoType;
        cell.color = (minoType == CellType::Gold) ? Colors::PieceGold :
                     (minoType == CellType::Bomb) ? Colors::PieceBomb :
                     (minoType == CellType::Jelly) ? Colors::PieceJelly : pieceColor;
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
    if (clearResult.linesCount > 0) {
        // Line clear audio
        app.GetSoundSynth().PlayLineClear(clearResult.linesCount);

        // Screen shake trauma
        float trauma = (clearResult.linesCount >= 4) ? 0.55f : (0.15f * static_cast<float>(clearResult.linesCount));
        m_screenEffects.AddTrauma(trauma);
        if (clearResult.linesCount >= 4) {
            m_screenEffects.TriggerFlash(Colors::TextAccent, 0.4f);
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
        std::string popupText = (clearResult.linesCount >= 4) ? "+800 TETRIS!" : ("+" + std::to_string(clearResult.linesCount * 100));
        if (clearResult.coinsGenerated > 0) {
            popupText += " (+$" + std::to_string(clearResult.coinsGenerated) + ")";
        }
        Vector2 centerPlayfield = { PLAYFIELD_X + gridWidthPx * 0.5f, PLAYFIELD_Y + CELL_SIZE * 6.0f };
        m_particles.AddPopup(popupText, centerPlayfield, (clearResult.linesCount >= 4) ? GOLD : Colors::TextAccent, (clearResult.linesCount >= 4) ? 1.4f : 1.0f);

        // Register with run manager
        m_runManager.RegisterLineClear(clearResult.linesCount, clearResult.linesCount >= 4, ctx);

        // Check if floor cleared and draft pending
        if (m_runManager.IsDraftPending()) {
            m_runManager.ConsumeDraftFlag();
            app.GetSoundSynth().PlayLevelUp();
            m_screenEffects.TriggerFlash(GOLD, 0.5f);
            app.GetStateManager().PushOverlay(app, std::make_unique<CardDraftState>(m_runManager.GetFloor()));
            return;
        }
    }

    // Spawn next piece
    SpawnNextPiece(app);
}

void PlayState::HandleMovementInput(GameApp& app, float dt) {
    // 1. DAS / ARR Horizontal Movement
    int currentDir = 0;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) currentDir = -1;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) currentDir = +1;

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
    if (m_isPaused) {
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
            m_isPaused = false;
        }
        return;
    }

    HandleMovementInput(app, GetFrameTime());
}

void PlayState::Update(GameApp& app, float dt) {
    if (m_isPaused) return;

    m_grid->Update(dt);
    m_particles.Update(dt);
    m_screenEffects.Update(dt);

    // Calculate effective fall interval (accelerated on Soft Drop)
    float fallInterval = m_runManager.GetFallInterval();
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        fallInterval /= SOFT_DROP_FACTOR;
    }

    m_activePiece.Update(dt, fallInterval, *m_grid, &app.GetEventBus());

    // Check if piece locked via lock delay
    if (m_activePiece.IsLocked()) {
        HandlePieceLock(app);
    }
}

void PlayState::Render(GameApp& /*app*/) {
    ClearBackground(Colors::BgDark);

    m_renderer.RenderGameHUD(
        *m_grid,
        m_activePiece,
        m_spawner,
        m_runManager,
        m_particles,
        m_screenEffects,
        m_showDebugPhysics
    );

    if (m_isPaused) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.6f));
        const char* pauseText = "GAME PAUSED";
        int w = MeasureText(pauseText, 36);
        DrawText(pauseText, (WINDOW_WIDTH - w) / 2, WINDOW_HEIGHT / 2 - 20, 36, Colors::TextWhite);
        const char* resumeText = "PRESS [P] OR [ESC] TO RESUME";
        int rw = MeasureText(resumeText, 14);
        DrawText(resumeText, (WINDOW_WIDTH - rw) / 2, WINDOW_HEIGHT / 2 + 30, 14, Colors::TextDim);
    }
}

} // namespace TetroShift
