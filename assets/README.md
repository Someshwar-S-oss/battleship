# Assets Directory

This directory contains game assets like fonts, sounds, and textures.

## Directory Structure

```
assets/
├── fonts/          # Font files (.ttf, .otf)
├── sounds/         # Sound effects (.wav, .ogg) [future]
└── textures/       # Sprite sheets, icons [future]
```

## Fonts

Place your TrueType font files here. The game will look for:
- `assets/fonts/arial.ttf` (or any .ttf file)

### Recommended Pixel Art Fonts

Free fonts that work great with the pixel art style:

1. **Press Start 2P**
   - Download: https://fonts.google.com/specimen/Press+Start+2P
   - License: Open Font License
   - Perfect for retro games

2. **VT323**
   - Download: https://fonts.google.com/specimen/VT323
   - License: Open Font License
   - Terminal-style monospace

3. **Pixelify Sans**
   - Download: https://fonts.google.com/specimen/Pixelify+Sans
   - License: Open Font License
   - Modern pixel font

4. **Silkscreen**
   - Download: https://fonts.google.com/specimen/Silkscreen
   - License: Open Font License
   - Classic pixelated look

### Installation

1. Download a font from the links above
2. Extract the .ttf file
3. Copy it to `assets/fonts/`
4. Optionally rename to `arial.ttf` for automatic detection

### Using System Fonts

If no font is found in `assets/fonts/`, the game automatically tries:
- **Windows**: `C:\Windows\Fonts\arial.ttf`
- **Linux**: `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf`
- **macOS**: System fonts

## Sounds (Future Feature)

When adding sound effects:
```
assets/sounds/
├── hit.wav      # Ship hit sound
├── miss.wav     # Water splash
├── sink.wav     # Ship sinking
├── place.wav    # Ship placement
└── victory.wav  # Game won
```

## Textures (Future Feature)

For sprite-based ships:
```
assets/textures/
├── ships.png           # Sprite sheet
├── water_animation.png # Animated water
└── explosions.png      # Particle effects
```

## License Notes

Make sure any assets you add are:
- Created by you, OR
- Licensed for use (check the license), OR
- Public domain

Always credit asset creators in your README.

---

**Need help?** Check `README_GUI.md` for more information.
