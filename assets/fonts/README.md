# 🔤 TetroShift Cyberpunk Vector Fonts

This directory hosts TrueType / OpenType (`.ttf` / `.otf`) vector fonts for *TetroShift // MorphoTetris*.

## Supported Font Layouts:
1. **Title / Display Font:**
   - `Orbitron-Bold.ttf` or `orbitron_bold.ttf` or `orbitron.ttf`
   - Rendered at 64px with bilinear filtering and multi-pass neon halo drop-shadows.

2. **Body / UI Font:**
   - `ShareTechMono-Regular.ttf` or `share_tech_mono.ttf`
   - Rendered at 48px with bilinear filtering for crisp descriptions, buttons, and badges.

3. **Mono / Digital Stats Font:**
   - `PressStart2P-Regular.ttf` or `press_start_2p.ttf`
   - Rendered at 48px for matrix coords, timer, and score numbers.

## Fallback Architecture:
- If custom fonts are placed here, `FontManager` loads them at high resolution with `LoadFontEx()` and applies bilinear filtering.
- If fonts are absent, `FontManager` automatically and transparently falls back to Raylib's built-in default font.
