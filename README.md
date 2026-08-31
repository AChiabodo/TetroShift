# TetroShift // MorphoTetris

> Motore di gioco Tetris Roguelike in C++20 con fisica Soft-Body, geometrie non-standard, post-processing shader GPU e sintesi audio procedurale basato su Raylib 5.0.

---

## Indice dei Contenuti

1. Panoramica del Progetto
2. Architettura del Software e Design Pattern
3. Modalita di Gioco
4. Meccaniche di Gioco e Fisica Soft-Body
5. Geometrie di Matrice Non-Standard
6. Pipeline Grafica, Shader GPU e Tipografia Vettoriale
7. Architettura Audio Ibrida
8. Sistema di Salvataggio e Meta-Progressione
9. Controlli e Mappatura Tasti
10. Requisiti di Sistema e Compilazione
11. Struttura del Repository

---

## 1. Panoramica del Progetto

TetroShift (MorphoTetris) e una reinterpretazione contemporanea del classico puzzle game a caduta di blocchi, sviluppata nativamente in C++20. Il progetto integra la precisione competitiva delle specifiche ufficiali Tetris (Super Rotation System, buffer di lock delay, DAS/ARR) con dinamiche roguelike deckbuilding, deformazioni elastiche fisiche basate su integrazione Verlet e matrici di gioco a topologia variabile (ortogonale, esagonale e radiale cilindrica).

Il motore e progettato con principi di modularita rigida, memoria sicura tramite idiomi RAII e zero allocazioni dinamiche nei percorsi critici di aggiornamento e rendering a 60 FPS.

---

## 2. Architettura del Software e Design Pattern

Il codice e organizzato in moduli indipendenti e disaccoppiati secondo le migliori pratiche dell'ingegneria del software:

- **State Pattern (`StateManager`, `IState`):** Gestione a stack degli stati applicativi (`TitleState`, `PlayState`, `CardDraftState`, `InRunShopState`, `GameOverState`).
- **Observer / Event-Driven Architecture (`EventBus`):** Bus eventi sincrono basato su tipi statici C++ per notificare linee completate, hard drop, lock di pezzi, transizioni di piano ed esplosioni senza accoppiamenti diretti.
- **Bridge & Polymorphism (`IGrid`):** Interfaccia astratta per disaccoppiare interamente le regole di caduta, collisione e rendering dalla geometria dello spazio di coordinate sottostante.
- **Data-Driven Progression (`CardDatabase`, `SaveManager`):** Configurazione dichiarativa di carte reliquia, modificatori fisici e serializzazione dello stato di run in formato JSON strutturato.
- **Resource Management RAII:** Incapsulamento completo di texture offscreen, shader GLSL, font vettoriali e flussi audio con rilascio automatico delle risorse.

---

## 3. Modalita di Gioco

Il gioco offre quattro modalita distinte per soddisfare sia il gioco competitivo arcade sia l'esplorazione sperimentale delle meccaniche:

### Roguelike Campaign (Modalita Principale)
- Avanzamento a piani con obiettivi di linee crescenti (Floor 1: 6 linee, Floor 2: 8 linee, ecc.).
- Selezione Draft a fine piano: scelta tra 3 carte reliquia con rarita variabile (Common, Rare, Epic, Legendary, Cursed).
- Negozio interno (In-Run Shop) per spendere crediti energetici accumulati in modificatori, riparazioni o rimozione maledizioni.
- Afflizioni ambientali generate proceduralmente (Hazard System: nebbia magnetica, gravita compressa, criogenia, sovratensione).

### Endless Matrix Marathon
- Modalita arcade classica pura, priva di modificatori roguelike o carte passive.
- Curva di velocita standard SRS con transizione automatica di livello ogni 10 linee completate (Livelli 1-20).
- Tracciamento dei record personali e graduatoria locale dedicata.

### Daily Protocol (Sfida Giornaliera)
- Partita roguelike con seed crittografico generato deterministicamente dalla data di sistema (formato YYYYMMDD).
- Condizioni di partenza e sequenze di tetramini identiche a livello globale per tutti i giocatori.

### Physics & Training Sandbox
- Laboratorio interattivo per collaudo in tempo reale di pezzi e proprieta fisiche.
- Strumenti live: commutazione istantanea della geometria di griglia (tasto F8), regolazione dell'elasticita delle molle da 0.2x a 3.0x (tasto F9), blocco della gravita (0G Mode, tasto F7) e generazione di mino speciali a scelta (Solid, Bomb, Gold, Jelly, Sand).

---

## 4. Meccaniche di Gioco e Fisica Soft-Body

### Super Rotation System (SRS) e Manovrabilita
- **Tabelle Wall-Kick Complete:** Risoluzione delle collisioni di rotazione su 4 stati (0, R, 2, L) con matrici kick dedicate per tetramini standard (J, L, S, T, Z) e pezzo I.
- **Delayed Auto Shift (DAS) & Auto Repeat Rate (ARR):** Spostamento orizzontale reattivo a frame rate indipendente con pre-ritardo e ripetizione continua.
- **Lock Delay con Reset Cap:** Timer di 0.5 secondi per riposizionamento prima del blocco definitivo, limitato a 15 movimenti o rotazioni per prevenire stalli infiniti.
- **Hold Queue & Ghost Piece:** Proiezione dinamica della traiettoria di caduta calcolata geometricamente per ciascuna topologia di griglia.

### Reticoli Elastici Spring-Mass Verlet
I tetramini non sono entita rigide ma corpi deformabili composti da nodi con integrazione Verlet (`VerletNode`) connessi da vincoli elastici smorzati (`SpringConstraint`). Il sistema reagisce con deformazioni visive realistiche:
- **Impulso di Movimento:** Deformazione laterale proporzionale alla velocita di traslazione.
- **Coppia di Rotazione:** Torsione angolare elastica durante le rotazioni SRS.
- **Compressione da Impatto (Squish):** Schiacciamento verticale calcolato in base all'altezza di caduta nell'Hard Drop.

### Tipologie Speciali di Mino
- **Solid:** Blocco standard strutturale.
- **Bomb:** Innesca reazioni a catena distruttive ad area circolare/esagonale quando la riga viene completata.
- **Gold:** Genera crediti energetici extra all'eliminazione per l'economia di gioco.
- **Jelly:** Aumenta marcatamente l'elasticita e il rimbalzo del pezzo durante il controllo.
- **Sand:** Sottoposto a fisica granulare; alla caduta i granelli scivolano diagonalmente negli spazi vuoti sottostanti secondo regole di automi cellulari.

---

## 5. Geometrie di Matrice Non-Standard

TetroShift supera la tradizionale griglia rettangolare implementando tre modelli geometrici concreti sotto l'interfaccia polimorfica `IGrid`:

```
                       +--------------+
                       |    IGrid     |
                       +------+-------+
                              |
        +---------------------+---------------------+
        |                     |                     |
        v                     v                     v
+---------------+     +---------------+     +---------------+
| OrthogonalGrid|     | HexagonalGrid |     |  RadialGrid   |
| 10x20 Standard|     | 60° Honeycomb |     | 360° Polar    |
+---------------+     +---------------+     +---------------+
```

### OrthogonalGrid (Matrice Standard 10x20)
- Spazio cartesiano classico a 10 colonne per 20 righe visibili, piu 4 righe di buffer superiore.
- Eliminazione standard per righe orizzontali piene e caduta gravitazionale dei blocchi residui.

### HexagonalGrid (Matrice Esagonale a Nido d'Ape)
- Spazio di coordinate assiali sfalsate (offset colonna sulle righe dispari).
- Topologia a 6 vicini: propagazione delle forze fisiche, scivolamento della sabbia su vertici a 60° ed esplosioni radiali a grafo.
- Eliminazione multi-asse lungo le direttrici diagonali e orizzontali.
- Rendering vettoriale nativo con poligoni regolari a 6 vertici e beveling 3D.

### RadialGrid (Matrice Radiale Cilindrica a 360°)
- Coordinate polari cilindriche (theta, r) composte da 16 settori angolari e 12 anelli concentrici.
- Wrap-Around Angolare Continuo: assenza di muri laterali; i pezzi ruotano e traslano a 360° attorno alla singolarita centrale.
- Eliminazione ad Anello Concentrico (Ring Clear): completamento di un intero anello a 360° con implosione centripeta verso il nucleo energetico.
- Rendering curvilineo con proiezione polare dei mino orbitali.

---

## 6. Pipeline Grafica, Shader GPU e Tipografia Vettoriale

### Shader Post-Processing GLSL (`assets/shaders/crt_bloom.fs`)
L'intero gioco viene renderizzato offscreen su un buffer `RenderTexture2D` ad alta precisione e post-processato su GPU tramite uno shader custom:
- **Neon Bloom Glow:** Estrazione delle alte frequenze luminose e sfocatura additiva calibrata.
- **Aberrazione Cromatica Dinamica:** Sdoppiamento dei canali RGB proporzionale all'intensita degli impatti sismici (screen trauma).
- **Filtro CRT e Scanlines:** Maschera di scansione orizzontale a fosfori e curvatura a barilotto arcade configurabile (attivabile/disattivabile con F4).

### Gestore Tipografico Vettoriale (`FontManager`)
- Caricamento dinamico di font TTF/OTF vettoriali da `assets/fonts/` (Orbitron, Share Tech Mono, Press Start 2P).
- Rasterizzazione ad alta densita (48px - 64px) con filtraggio bilineare della texture per prevenire artefatti di scalettatura.
- Metodo di disegno con bagliore neon a quadruplo offset per titoli ed elementi HUD prioritari.
- Fallback automatico trasparente sul font bitmap predefinito in assenza dei file binari.

---

## 7. Architettura Audio Ibrida

Il comparto sonoro e gestito attraverso un'architettura a doppio canale che unisce sintesi autonoma e streaming multitraccia:

### Sintetizzatore Software Procedurale (`SoundSynth`)
- Generazione in tempo reale a 22.05 kHz di forme d'onda pure (sinusoidale, quadra, triangolare, rumore bianco) modulate con inviluppi ADSR.
- Copertura di tutti i feedback sonori: rotazioni, lock, hard drop, eliminazioni singole/doppie/triple/Tetris, esplosioni di bombe e acquisto carte.
- Funzionamento garantito al 100% senza dipendenze da file audio esterni su disco.

### Motore Musicale Streaming Multi-Traccia (`MusicManager`)
- Riconoscimento e caricamento automatico di file OGG Vorbis polifonici da `assets/music/` per le 8 tracce della colonna sonora.
- Dissolvenza incrociata fluida (Crossfade) e modulazione in tempo reale del fattore di urgenza (Lowpass Filter / Pitch) all'avvicinarsi della cima della matrice.

---

## 8. Sistema di Salvataggio e Meta-Progressione

### Gestione dei Salvataggi (`SaveManager`)
- **Tre Slot di Run Indipendenti:** Possibilita di salvare e sospendere la sessione corrente dal menu di pausa per riprenderla in un secondo momento.
- **Profilo Giocatore e Rango:** Accumulo di punti esperienza (EXP), avanzamento di livello pilota, titoli di rango e crediti energetici.
- **Albo d'Oro Locale (High Scores):** Tabella dei migliori punteggi con data, modalita di gioco, linee e badge di rendimento.
- **Impostazioni Persistenti:** Salvataggio su file JSON di volumi audio, intensita screen shake, filtri grafici e velocita di input.

---

## 9. Controlli e Mappatura Tasti

### Controlli di Gioco Standard

| Tasto | Azione |
| :--- | :--- |
| **Freccia Sinistra / A** | Traslazione orizzontale a sinistra (con DAS/ARR) |
| **Freccia Destra / D** | Traslazione orizzontale a destra (con DAS/ARR) |
| **Freccia Giu / S** | Soft Drop (Caduta rapida controllata) |
| **Barra Spaziatrice** | Hard Drop (Caduta istantanea con impatto sismico) |
| **Freccia Su / W / X** | Rotazione in senso orario (SRS CW) |
| **Z / Ctrl Sinistro** | Rotazione in senso antiorario (SRS CCW) |
| **C / Shift Sinistro** | Hold (Scambio con pezzo di riserva) |
| **Tasti 1 / 2** | Attivazione Abilita Speciali e Carte Attive |
| **P / ESC** | Menu di Pausa / Riprendi Missione |

### Tasti Funzione e Strumenti Sandbox

| Tasto | Funzione |
| :--- | :--- |
| **F1** | Mostra/Nascondi nodi fisici Verlet e reticolo molle (Debug Wireframe) |
| **F2** | Apertura immediata della schermata di selezione carte Card Draft (Debug) |
| **F3** | Eliminazione forzata istantanea di una riga per test particelle (Debug) |
| **F4** | Attiva/Disattiva shader CRT, scanlines e curvatura a barilotto |
| **F5** | Riavvio immediato della sessione di gioco corrente |
| **F6** | Ciclo tipologia di mino selezionato in modalita Sandbox |
| **F7** | Attiva/Disattiva modalita gravita zero (0G Mode) in Sandbox |
| **F8** | Commutazione live della geometria di matrice (Orthogonal -> Hexagonal -> Radial) |
| **F9** | Regolazione ciclica dell'elasticita globale delle molle (0.5x -> 3.0x) |
| **F10** | Svuotamento completo della matrice in Sandbox |
| **F11** | Inserimento riga di disturbo (Garbage Row) per test di collasso |

---

## 10. Requisiti di Sistema e Compilazione

### Prerequisiti
- **Compilatore C++20:** Microsoft Visual C++ (MSVC 2022 o successivo), GCC 11+ o Clang 13+.
- **CMake:** Versione 3.22 o superiore.
- **Sistema Operativo:** Windows 10/11, Linux, macOS (OpenGL 3.3 supportato).

### Compilazione da Riga di Comando

```bash
# 1. Configurazione del progetto tramite CMake
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

# 2. Compilazione dell'eseguibile ottimizzato
cmake --build build --config Release

# 3. Esecuzione del binario
# Su Windows:
./build/Release/TetroShift.exe

# Su Linux / macOS:
./build/TetroShift
```

---

## 11. Struttura del Repository

```
TetroShift/
├── assets/                  # Risorse grafiche, sonore, font e shader
│   ├── fonts/               # Font vettoriali TrueType / OpenType
│   ├── music/               # File audio OGG Vorbis per lo streaming musicale
│   └── shaders/             # Shader GLSL per post-processing (crt_bloom.fs)
├── src/
│   ├── audio/               # Sintesi procedurale e gestione flussi sonori
│   │   ├── MusicManager.*   # Motore musicale con crossfade e filtri dinamici
│   │   ├── MusicTrack.hpp   # Definizioni dei brani e generi sonori
│   │   └── SoundSynth.*     # Sintetizzatore software real-time ad onde pure
│   ├── core/                # Nucleo applicativo, eventi e persistenza
│   │   ├── Constants.hpp    # Dimensioni finestra, palette colori e parametri fisici
│   │   ├── EventBus.hpp     # Bus eventi sincrono per disaccoppiamento moduli
│   │   ├── GameApp.*        # Entry point del ciclo principale e gestione finestre
│   │   └── SaveManager.*    # Serializzatore JSON, profilo pilota e slot di run
│   ├── grid/                # Sottosistema geometrie matrice polimorfiche
│   │   ├── GeometryTypes.hpp# Enumerazione tipologie geometriche
│   │   ├── HexagonalGrid.*  # Matrice a nido d'ape a 6 vicini e coordinate assiali
│   │   ├── IGrid.hpp        # Interfaccia pura per griglie polimorfiche
│   │   ├── OrthogonalGrid.* # Matrice cartesiana standard 10x20
│   │   └── RadialGrid.*     # Matrice cilindrica polare a 360° con nucleo singolarita
│   ├── physics/             # Motore di dinamica elastica Soft-Body
│   │   ├── SoftBodyMesh.*   # Reticolo di nodi Verlet e vincoli a molla
│   │   ├── SpringConstraint.* # Risoluzione dei vincoli elastici con smorzamento
│   │   └── VerletNode.hpp   # Integratore di posizione e velocita per singolo nodo
│   ├── piece/               # Tetramini e logiche di manovra
│   │   ├── ActivePiece.*    # Controllo pezzo attivo, lock delay e SRS kick
│   │   ├── Spawner.*        # Generatore con sacca casuale a 7 pezzi e seed giornaliero
│   │   └── Tetromino.hpp    # Tabelle geometriche SRS dei tetramini
│   ├── render/              # Pipeline grafica e post-processing
│   │   ├── FontManager.*    # Caricatore font vettoriali con filtraggio bilineare
│   │   ├── MenuRenderer.*   # Disegno menu e bottoni con tipografia e glow
│   │   ├── ParticleSystem.* # Emettitore di particelle per esplosioni e polvere
│   │   ├── Renderer.*       # Composizione grafica completa dell'HUD e del playfield
│   │   └── ScreenEffects.*  # Gestione trauma screen shake e shader GPU
│   ├── roguelike/           # Meccaniche deckbuilding e pericoli ambientali
│   │   ├── CardDatabase.*   # Catalogo di carte reliquia e abilita attive
│   │   ├── CardInventory.*  # Inventario reliquie, monete e consumabili del pilota
│   │   ├── HazardManager.*  # Generatore di afflizioni ambientali di piano
│   │   └── RunManager.*     # Calcolo punteggi, moltiplicatori e avanzamento piani
│   └── states/              # Macchina a stati dell'applicazione
│       ├── CardDraftState.* # Stato di selezione carte premio di fine piano
│       ├── GameOverState.*  # Schermata di resoconto missione e calcolo EXP
│       ├── InRunShopState.* # Negozio interno per acquisto modificatori
│       ├── IState.hpp       # Interfaccia per gli stati applicativi
│       ├── MenuTypes.hpp    # Strutture dati per menu, profili e impostazioni
│       ├── PlayState.*      # Stato principale del loop di gioco
│       ├── StateManager.*   # Gestore dello stack degli stati applicativi
│       └── TitleState.*     # Schermata iniziale e selezione modalita
├── CMakeLists.txt           # File di configurazione build CMake
└── README.md                # Documentazione tecnica del progetto
```
