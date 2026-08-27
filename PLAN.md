# 📐 PLAN.md — Progetto "TetroShift / MorphoTetris" (Raylib C++)

Un clone innovativo di Tetris sviluppato in **C++20** e **Raylib 5.x**, che unisce meccaniche di **fisica e deformabilità**, un sistema **Roguelike a modificatori/carte**, e un'architettura modulare predisposta per **geometrie non standard** (ortogonale, esagonale, radiale).

---

## 📑 Indice
1. [Visione & Obiettivi del Progetto](#1-visione--obiettivi-del-progetto)
2. [Stack Tecnologico & Dipendenze](#2-stack-tecnologico--dipendenze)
3. [Architettura del Software & Design Patterns](#3-architettura-del-software--design-patterns)
4. [I Tre Pilastri di Innovazione](#4-i-tre-pilastri-di-innovazione)
   - [4.1 Fisica & Pezzi Deformabili / Instabili](#41-fisica--pezzi-deformabili--instabili)
   - [4.2 Sistema Roguelike (Carte, Reliquie, Economia)](#42-sistema-roguelike-carte-reliquie-economia)
   - [4.3 Astrazione Geometrica della Griglia](#43-astrazione-geometrica-della-griglia)
5. [Struttura delle Directory del Progetto](#5-struttura-delle-directory-del-progetto)
6. [Dettaglio dei Moduli e Interfacce C++](#6-dettaglio-dei-moduli-e-interfacce-c)
7. [Tabella delle Carte e Modificatori (Esempi MVP)](#7-tabella-delle-carte-e-modificatori-esempi-mvp)
8. [Roadmap di Sviluppo in Fasi (Milestone & Task)](#8-roadmap-di-sviluppo-in-fasi-milestone--task)
9. [Linee Guida di Build, Test e Debugging](#9-linee-guida-di-build-test-e-debugging)

---

## 1. Visione & Obiettivi del Progetto

Il progetto mira a reinventare il gameplay classico di Tetris mantenendone la reattività immediata ("game feel" arcade), ma introducendo profondità strategica e dinamismo fisico:
* **Feeling Dinamico:** I pezzi non sono semplici blocchi rigidi; possiedono massa, inerzia e un modello elastico (Spring-Mass / Verlet) che reagisce agli urti, rotazioni rapide e incastri stretti.
* **Profondità Roguelike:** Ogni partita è un "run" diviso in floor/obiettivi. Completare linee o raggiungere traguardi sblocca draft di carte che alterano radicalmente le regole (gravità multidirezionale, pezzi gelatinosi, blocchi bomba, moltiplicatori di punteggio).
* **Architettura a Griglia Astratta:** Il motore logico è disaccoppiato dalle coordinate cartesiane 2D, permettendo di passare dalla griglia standard 10x20 a griglie esagonali o circolari concentriche tramite interfacce generiche.

---

## 2. Stack Tecnologico & Dipendenze

| Componente | Scelta | Motivazione |
| :--- | :--- | :--- |
| **Linguaggio** | **C++20** | Performance native, allocazione memoria controllata, `std::span`, `concepts`, `smart pointers`. |
| **Libreria Multimediale** | **Raylib 5.0+** | Rendering 2D/3D hardware-accelerated, gestione input a bassissima latenza, gestione audio senza overhead. |
| **Build System** | **CMake 3.22+** | Portabilità multipiattaforma (Windows, Linux, macOS, WebAssembly/Emscripten). |
| **Sistema Fisico** | **Custom 2D Verlet / Spring-Damper** | Zero dipendenze pesanti esterne; controllo millimetrico su rimbalzi, squish e deformazioni visive/collisioni. |
| **UI Framework** | **Raygui / Custom Immediate GUI** | Interfaccia snella per menu, shop e draft carte integrata nel loop di Raylib. |

---

## 3. Architettura del Software & Design Patterns

L'architettura segue una combinazione di **Event-Driven Architecture**, **Strategy Pattern** (per le geometrie e modificatori) e **State Pattern** (per i flussi di gioco).

```mermaid
graph TD
    GameApp[GameApp - Main Loop] --> StateMachine[GameStateManager]
    StateMachine --> TitleState[Title / Menu State]
    StateMachine --> PlayState[PlayState]
    StateMachine --> DraftState[Card Draft State]
    StateMachine --> ShopState[Shop / Upgrade State]
    
    PlayState --> GridEngine[GridEngine (IGrid)]
    PlayState --> PieceEngine[PieceEngine (Tetromino & Physics)]
    PlayState --> CardSystem[Card & Modifier System]
    PlayState --> EventBus[EventBus]
    PlayState --> RenderPipeline[RenderPipeline & Shaders]
    PlayState --> AudioSystem[AudioSystem]
    
    GridEngine -.-> OrthoGrid[OrthogonalGrid 10x20]
    GridEngine -.-> HexGrid[HexagonalGrid (Future)]
    GridEngine -.-> RadialGrid[RadialGrid (Future)]
    
    EventBus --> CardSystem
    CardSystem -->|Hooks / Mutators| PieceEngine
    CardSystem -->|Hooks / Mutators| GridEngine
```

---

## 4. I Tre Pilastri di Innovazione

### 4.1 Fisica & Pezzi Deformabili / Instabili
* **Modello Soft-Body (Spring-Mass / Verlet):** Ogni minomino (blocco costituente del tetramino) o l'intero pezzo è modellato come un insieme di nodi punto con masse collegate da molle e smorzatori elastici (*spring-damper lattice*).
* **Squish & Wobble:** Quando un pezzo impatta il pavimento o ruota a filo muro ("wall kick"), i nodi oscillano realisticamente prima di stabilizzarsi.
* **Instabilità e Massa:** I pezzi possono avere pesi differenti (es. Blocco d'Acciaio pesante, Blocco Gelatina rimbalzante, Blocco Sabbioso che si disgrega dopo il lock).
* **Locking a Rilassamento:** Il bloccaggio del pezzo può avvenire quando l'energia cinetica delle molle scende sotto una soglia di tolleranza.

### 4.2 Sistema Roguelike (Carte, Reliquie, Economia)
* **Struttura della Run:**
  - Piani / Livelli con obiettivi (es. *Sopravvivi a 30 pezzi*, *Pulisci 12 linee*, *Sconfiggi il Boss della Gravità*).
  - Dopo ogni livello o dopo $N$ linee pulite contemporaneamente (Tetris/Triple) si attiva la fase **Draft**: scelta tra 3 carte casuali di diversa rarità (*Common, Rare, Epic, Legendary, Cursed*).
* **Tipologie di Modificatori:**
  - **Passivi (Reliquie):** Modifiche permanenti alla run (es. *"Linea di Rame: Ogni linea pulita genera 5 monete extra"*, *"Elasticità +30%"*).
  - **Attivi / Consumabili:** Abilità ricaricabili tramite linee o mana (es. *"Bomba di Gravità: distrugge 3x3 blocchi"*, *"Hold Extra"*).
  - **Maledizioni (Risk/Reward):** Bonus enormi a costo di penalità (es. *"Punteggio triplicato, ma la velocità di caduta aumenta del 50%"*).
* **Economia:** Punti ed Essenze raccolti durante il gioco permettono di acquistare consumabili nel Negozio tra un livello e l'altro.

### 4.3 Astrazione Geometrica della Griglia
Per supportare future geometrie senza riscrivere il core loop:
* **Interfaccia `IGrid`:**
  - Coordinate logiche generalizzate `GridCoord` (es. cubiche/assiali per esagoni, $(r, \theta)$ per radiali, $(x,y)$ per ortogonali).
  - Metodi astratti per: `IsValid(coord)`, `IsOccupied(coord)`, `SetCell(coord, cellData)`, `FindCompleteLines()`, `ClearLines(lines)`, `ToWorldPos(coord)`.
* **MVP:** `OrthogonalGrid` ($10 \times 20$) implementa `IGrid` con coordinate $(x,y)$ classiche.

---

## 5. Struttura delle Directory del Progetto

```text
TetrisRaylib/
├── CMakeLists.txt
├── README.md
├── PLAN.md
├── assets/
│   ├── fonts/
│   │   └── press_start.ttf
│   ├── shaders/
│   │   ├── crt_bloom.fs
│   │   └── deformation.vs
│   ├── sounds/
│   │   ├── move.wav
│   │   ├── rotate.wav
│   │   ├── drop.wav
│   │   ├── clear.wav
│   │   └── soft_impact.wav
│   └── music/
│       └── theme_synth.ogg
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── GameApp.hpp / .cpp
│   │   ├── GameStateManager.hpp / .cpp
│   │   ├── EventBus.hpp / .cpp
│   │   └── Constants.hpp
│   ├── states/
│   │   ├── IGameState.hpp
│   │   ├── TitleState.hpp / .cpp
│   │   ├── PlayState.hpp / .cpp
│   │   ├── CardDraftState.hpp / .cpp
│   │   ├── ShopState.hpp / .cpp
│   │   └── GameOverState.hpp / .cpp
│   ├── grid/
│   │   ├── IGrid.hpp
│   │   ├── GridCoord.hpp
│   │   ├── Cell.hpp
│   │   ├── OrthogonalGrid.hpp / .cpp
│   │   └── GeometryTypes.hpp
│   ├── physics/
│   │   ├── VerletNode.hpp
│   │   ├── SpringConstraint.hpp
│   │   ├── SoftBodyMesh.hpp / .cpp
│   │   └── PhysicsEngine.hpp / .cpp
│   ├── piece/
│   │   ├── TetrominoType.hpp
│   │   ├── TetrominoDefinition.hpp
│   │   ├── ActivePiece.hpp / .cpp
│   │   └── PieceSpawner.hpp / .cpp
│   ├── roguelike/
│   │   ├── Card.hpp
│   │   ├── CardDatabase.hpp / .cpp
│   │   ├── ModifierRegistry.hpp / .cpp
│   │   ├── Inventory.hpp / .cpp
│   │   └── RunManager.hpp / .cpp
│   ├── render/
│   │   ├── Renderer.hpp / .cpp
│   │   ├── ParticleSystem.hpp / .cpp
│   │   └── ScreenEffects.hpp / .cpp
│   └── audio/
│       └── AudioManager.hpp / .cpp
```

---

## 6. Dettaglio dei Moduli e Interfacce C++

### 6.1 `IGrid.hpp` — Astrazione Griglia
```cpp
#pragma once
#include "GridCoord.hpp"
#include "Cell.hpp"
#include <vector>
#include <raylib.h>

struct LineClearResult {
    int linesCleared = 0;
    std::vector<GridCoord> clearedCells;
    float scoreMultiplier = 1.0f;
};

class IGrid {
public:
    virtual ~IGrid() = default;
    virtual void Initialize(int width, int height) = 0;
    virtual void Clear() = 0;
    virtual bool IsValidCoord(const GridCoord& coord) const = 0;
    virtual bool IsCellOccupied(const GridCoord& coord) const = 0;
    virtual const Cell& GetCell(const GridCoord& coord) const = 0;
    virtual void SetCell(const GridCoord& coord, const Cell& cell) = 0;
    
    virtual LineClearResult CheckAndClearLines() = 0;
    virtual Vector2 CoordToWorld(const GridCoord& coord, Vector2 gridOrigin, float cellSize) const = 0;
    virtual void Render(Vector2 gridOrigin, float cellSize) const = 0;
};
```

### 6.2 `SoftBodyMesh.hpp` — Nodi e Molle per Deformabilità
```cpp
#pragma once
#include <raylib.h>
#include <vector>

struct VerletPoint {
    Vector2 position;
    Vector2 previousPosition;
    Vector2 acceleration;
    float mass = 1.0f;
    bool isPinned = false;
};

struct Spring {
    int p1, p2;
    float restLength;
    float stiffness; // k
    float damping;
};

class SoftBodyMesh {
public:
    std::vector<VerletPoint> points;
    std::vector<Spring> springs;

    void Update(float dt, Vector2 gravity);
    void ApplyImpulse(Vector2 impulse);
    void TriggerSquish(Vector2 impactVelocity);
    void Render(Color color) const;
};
```

### 6.3 `Card.hpp` & `EventBus.hpp` — Sistema Roguelike
```cpp
#pragma once
#include <string>
#include <functional>
#include <raylib.h>

enum class CardRarity { Common, Rare, Epic, Legendary, Cursed };
enum class CardTarget { Grid, ActivePiece, Score, Spawner, Global };

struct CardContext {
    class PlayState* playState;
    class ActivePiece* piece;
    class IGrid* grid;
};

struct Card {
    std::string id;
    std::string title;
    std::string description;
    CardRarity rarity;
    CardTarget target;
    Color cardColor;
    
    // Hooks
    std::function<void(const CardContext&)> onAcquire;
    std::function<void(const CardContext&)> onPieceSpawn;
    std::function<void(const CardContext&, int linesCleared)> onLineClear;
    std::function<void(const CardContext&)> onPieceLock;
};
```

---

## 7. Tabella delle Carte e Modificatori (Esempi MVP)

| ID Carta | Nome | Rarità | Effetto Meccanico |
| :--- | :--- | :--- | :--- |
| `CARD_JELLY` | **Corpo Gelatinoso** | Common | Aumenta l'elasticità dei pezzi del 50%; i pezzi possono rimbalzare e infilarsi in fessure strette. |
| `CARD_HEAVY_IRON` | **Massa Titanica** | Rare | I pezzi cadono con massa quadrupla; al lock distruggono 1 riga sotto di loro se c'erano buchi. |
| `CARD_MAGNET_L` | **Attrazione Laterale** | Rare | Premendo un tasto, i blocchi orfani sulla griglia scivolano verso la parete più vicina. |
| `CARD_FISSION` | **Reazione a Catena** | Epic | Pulire 4 linee (Tetris) innesca un'esplosione che converte le 2 righe adiacenti in punti e le elimina. |
| `CARD_GHOST_PHASE` | **Fase Quantistica** | Epic | I pezzi possono attraversare blocchi già piazzati per i primi 0.5s dopo lo spawn. |
| `CARD_MIDAS` | **Tocco di Mida** | Legendary | Ogni 10° pezzo è d'oro: vale $5\times$ punti e ogni blocco pulito rilascia monete nello Shop. |
| `CARD_CURSE_CHAOS` | **Caos Gravitazionale** | Cursed | +100% Punti Run, ma ogni 15 secondi la direzione di caduta ruota di 90 gradi. |

---

## 8. Roadmap di Sviluppo in Fasi (Milestone & Task)

### 🔹 Fase 1: Setup Progetto & Core Tetris Classico (MVP Baseline)
- [ ] Configurazione ambiente CMake con Raylib 5.x integrata (FetchContent o submodule).
- [ ] Creazione del ciclo base `GameApp` e state machine (`GameStateManager`).
- [ ] Implementazione delle strutture `GridCoord`, `Cell` e `OrthogonalGrid` (10 colonne, 20 righe).
- [ ] Implementazione Tetromino standard (I, J, L, O, S, T, Z) con matrice di rotazione standard (SRS).
- [ ] Logica 7-bag randomizer, DAS (Delayed Auto Shift), ARR (Auto Repeat Rate) e Lock Delay.
- [ ] Rilevamento e pulizia linee con calcolo punteggio base.

### 🔹 Fase 2: Motore Fisico & Deformabilità (Soft-Body Layer)
- [ ] Implementazione del risolutore Verlet 2D e vincoli a molla (`SoftBodyMesh`).
- [ ] Connessione tra il `Tetromino` logico e il reticolo elastico visivo/fisico.
- [ ] Gestione degli urti con pavimento e pareti: calcolo del vettore impatto e oscillazione realistica.
- [ ] Implementazione dell'effetto "Gelatina / Deformazione al contatto".
- [ ] Visualizzazione rendering con poligoni Raylib deformati o shader mesh.

### 🔹 Fase 3: Architettura Roguelike & Sistema Carte
- [ ] Realizzazione dell'`EventBus` per notificare eventi: `OnPieceSpawn`, `OnMove`, `OnLock`, `OnLineClear`.
- [ ] Creazione del database carte (`CardDatabase`) con le prime 15-20 carte implementate.
- [ ] Creazione dell'interfaccia UI `CardDraftState` (selezione a 3 carte con animazioni e tooltip).
- [ ] Sistema di progressione a Livelli/Piani con contatore di obiettivi e negozio (`ShopState`).
- [ ] Gestione inventario reliquie passive e abilità attive.

### 🔹 Fase 4: "Game Juice", Shaders, Particelle & Audio
- [ ] Sistema di particelle Raylib per impatti, frammenti di blocchi distrutti e scie.
- [ ] Shader GLSL in post-processing: Bloom selettivo su blocchi neon, effetto CRT opzionale, screen shake dinamico.
- [ ] Integrazione audio Raylib: effetti sonori con modulazione di pitch in base all'altezza della griglia e combo.
- [ ] Transizioni fluide tra stati di gioco.

### 🔹 Fase 5: Estensibilità Geometrica (Proof of Concept)
- [ ] Dimostrazione dell'astrazione `IGrid`: creazione di una mini modalità con griglia a cilindro/radiale o esagonale.
- [ ] Test di regressione per garantire che l'ortogonale rimanga la modalità standard stabilissima.

---

## 9. Linee Guida di Build, Test e Debugging

### Compilazione rapida con CMake:
```bash
# Configurazione
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# Compilazione
cmake --build build --config Debug

# Esecuzione
./build/TetrisRaylib (o ./build/Debug/TetrisRaylib.exe su Windows)
```

### Strumenti di Debug In-Game (Fasti Debug Keys):
* `F1`: Mostra hitbox fisiche e nodi molle Verlet.
* `F2`: Forza apertura immediata del Card Draft.
* `F3`: Pulizia istantanea di 4 linee (test effetti e combo).
* `F4`: Toggle wireframe / shader post-processing.
* `F5`: Ricarica rapida dello stato di gioco.
