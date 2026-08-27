# 🕹️ TetroShift // MorphoTetris

> **Clone innovativo di Tetris Roguelike con Fisica Soft-Body in C++20 e Raylib 5.0**

---

## 🌟 Caratteristiche Principali

- **🧱 Super Rotation System (SRS) & Controlli Arcade Precisi:** Rotazioni con tabelle wall-kick SRS ufficiali (pezzi standard e pezzo I), Delayed Auto Shift (DAS), Auto Repeat Rate (ARR), Lock Delay con limite di reset e Ghost Piece.
- **✨ Motore Fisico & Soft-Body:** I tetramini non sono rigidi ma modellati con reticoli **Spring-Mass / Verlet** che reagiscono con squish, wobble e rimbalzi elastici a spostamenti, rotazioni e impatti.
- **🃏 Sistema Roguelike & Card Draft:**
  - Piani a obiettivi crescenti (es. *Raggiungi 6 linee per il Floor 1*).
  - Selezione Draft a 3 carte con rarità (*Common, Rare, Epic, Legendary, Cursed*).
  - Reliquie passive permanenti (es. *Corpo Gelatinoso*, *Massa Titanica*, *Tocco di Mida*, *Reticolo Esplosivo*, *Fase Quantistica*).
  - Abilità attive con cooldown in linee (*Attrazione Magnetica*, *Distorsione Temporale*).
  - Economia di monete per acquistare carte o rilanciare le scelte (*Reroll*).
- **🔊 Sintetizzatore Audio Procedurale Raylib:** Generazione procedurale di onde sinusoidali, triangolari e quadre a runtime per effetti sonori arcade 8-bit/synthwave a latenza zero (senza dipendenze da file audio esterni).
- **💥 Game Juice & Particelle:** Esplosioni di particelle alle linee completate, polvere d'impatto, screen shake non-lineare basato su trauma, flash dinamici e testi galleggianti per combo e punteggi.
- **📐 Architettura Astratta `IGrid`:** Disaccoppiamento completo tra logica di gioco, coordinate e rendering.

---

## 🎮 Controlli di Gioco

| Tasto | Azione |
| :--- | :--- |
| **Freccia Sinistra / A** | Sposta a sinistra (con DAS/ARR) |
| **Freccia Destra / D** | Sposta a destra (con DAS/ARR) |
| **Freccia Giù / S** | Soft Drop (Caduta rapida) |
| **Spazio** | Hard Drop (Caduta istantanea con impatto sismico) |
| **Freccia Su / W / X** | Ruota in senso orario (SRS CW) |
| **Z / Ctrl Sinistro** | Ruota in senso antiorario (SRS CCW) |
| **C / Shift Sinistro** | Hold / Scambia pezzo di riserva |
| **1 / 2** | Attiva le Abilità Speciali (es. Attrazione Magnetica) |
| **P / ESC** | Pausa / Riprendi |
| **F1** | Mostra Hitbox e Nodi/Molle Fisiche Verlet (Debug) |
| **F2** | Forza apertura immediata del Card Draft (Debug) |
| **F3** | Pulizia istantanea di una riga per test particelle/suoni (Debug) |
| **F4** | Attiva/Disattiva Overlay CRT & Scanlines |
| **F5** | Riavvia immediatamente la Run |

---

## 🛠️ Come Compilare ed Eseguire

### Prerequisiti
- **C++20** compatibile (MSVC 2022/2026, GCC 11+, Clang 13+)
- **CMake 3.22+**

### Build con CMake
```bash
# Configura il progetto
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

# Compila l'eseguibile
cmake --build build --config Release

# Avvia il gioco su Windows
./build/Release/TetroShift.exe
```
