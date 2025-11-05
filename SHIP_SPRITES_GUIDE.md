# Using Custom Ship Sprites

Your game now supports custom pixel art ship images! Here's how to use them.

## File Setup

### 1. Place your PNG files in the assets folder

Copy your ship images to: `assets/textures/`

### 2. Name your files correctly

The game looks for these specific file names:

```
assets/textures/
├── aircraft_carrier.png  (for Aircraft Carrier - 5 cells)
├── battleship.png        (for Battleship - 4 cells)
├── cruiser.png           (for Cruiser - 3 cells)
├── submarine.png         (for Submarine - 3 cells)
└── destroyer.png         (for Destroyer - 2 cells)
```

**Important:** File names must match exactly (lowercase, underscores)!

## Image Requirements

### Recommended Specifications

- **Format:** PNG with transparency
- **Orientation:** Horizontal (pointing right)
- **Aspect Ratio:** Wide rectangle
- **Size:** Any size works, but 160x40 pixels is ideal for crisp pixel art
- **Style:** Pixel art works best!

### Image Guidelines

1. **Horizontal Layout:** Draw your ships facing RIGHT →
   - The game will automatically rotate them for vertical placement
   
2. **Transparent Background:** Use PNG transparency so ocean shows through

3. **Proportions:**
   - Aircraft Carrier: Longest ship (5 cells)
   - Battleship: Second longest (4 cells)
   - Cruiser & Submarine: Medium (3 cells each)
   - Destroyer: Smallest (2 cells)

4. **Cell Size:** Each cell is 40x40 pixels on screen
   - Aircraft Carrier will be 200x40 pixels
   - Destroyer will be 80x40 pixels

### Example Dimensions

Good sizes for pixel art:
```
aircraft_carrier.png: 200x40 px (or any 5:1 ratio)
battleship.png:       160x40 px (or any 4:1 ratio)
cruiser.png:          120x40 px (or any 3:1 ratio)
submarine.png:        120x40 px (or any 3:1 ratio)
destroyer.png:         80x40 px (or any 2:1 ratio)
```

## How It Works

### Automatic Detection

When the game starts:
1. It tries to load all 5 ship textures
2. If **all** textures load successfully → Sprites enabled! ✓
3. If **any** texture fails → Falls back to colored rectangles

### Visual Features

With sprites enabled:
- ✓ Ships display as your custom images
- ✓ Automatically scaled to fit cells
- ✓ Automatically rotated for vertical placement
- ✓ Hit markers (red circles) drawn on top
- ✓ Sunk ships become gray and semi-transparent
- ✓ Works on both player and enemy boards

## Testing Your Sprites

### 1. Copy your images
```bat
copy my_aircraft_carrier.png assets\textures\aircraft_carrier.png
copy my_battleship.png assets\textures\battleship.png
REM ... etc for all 5 ships
```

### 2. Rebuild the project
```bat
build_msys2.bat
```

### 3. Run and check console
```bat
cd build
.\fleet_commander_gui.exe
```

Look for these messages:
```
Loaded texture: assets/textures/aircraft_carrier.png
Loaded texture: assets/textures/battleship.png
Loaded texture: assets/textures/cruiser.png
Loaded texture: assets/textures/submarine.png
Loaded texture: assets/textures/destroyer.png
Ship sprites enabled!
```

### If sprites don't appear:

Check console for warnings:
```
Warning: Could not load assets/textures/battleship.png
Using default colored rectangles for ships.
```

**Fix:** Make sure:
- Files are in correct folder
- File names are exact (lowercase, underscores)
- Files are valid PNG format
- Files are not corrupted

## Creating Pixel Art Ships

### Option 1: Use Free Pixel Art Software

- **Aseprite** (paid, but best for pixel art)
- **LibreSprite** (free, Aseprite fork)
- **GIMP** (free, has pixel art tools)
- **Piskel** (free, online: piskelapp.com)

### Option 2: Find Free Sprites

Search for "pixel art battleship sprite" or similar. Make sure they're free to use!

### Option 3: Simple Shapes

Even simple colored rectangles with details work:
1. Create 200x40 PNG for aircraft carrier
2. Fill with a ship color
3. Add some pixel art details (windows, deck, etc.)
4. Export as PNG

### Quick Example Template

For a simple pixel art destroyer (80x40):
```
■■■■■■■■■■■■■■■■  ← Top row (darker gray)
████████████████  ← Ship body (medium gray)
████████████████  
████████████████
■■■■■■■■■■■■■■■■  ← Bottom row (darker gray)
```

## Distribution

When sharing your game:
```
YourGame/
├── fleet_commander_gui.exe
├── *.dll (all DLL files)
└── assets/
    └── textures/
        ├── aircraft_carrier.png
        ├── battleship.png
        ├── cruiser.png
        ├── submarine.png
        └── destroyer.png
```

## Advanced Customization

### Adding Hit Effects to Sprites

The game already draws red circles on hits. To customize:
1. Edit `drawShipSprite()` in `GameGUI.cpp`
2. Change hit marker size, color, or shape

### Animated Sprites (Future)

To add animation:
1. Create sprite sheet with frames
2. Modify `drawShipSprite()` to cycle through frames
3. Use `sf::Clock` to time the animation

### Different Styles Per Board

To have different sprites for player vs computer:
1. Load two sets of textures
2. Pass different texture maps to player/computer BoardViews

## Troubleshooting

**Q: Sprites appear stretched or distorted**
A: Make sure your image aspect ratio matches ship length
   - 5:1 for aircraft carrier
   - 4:1 for battleship, etc.

**Q: Sprites are blurry**
A: The game uses `texture.setSmooth(false)` for pixel art.
   If images are blurry, check if they're high resolution.
   Use exact multiples of 40 pixels per cell for best results.

**Q: Only some ships show sprites**
A: All 5 textures must load. Check console for which files failed.

**Q: Game still shows colored rectangles**
A: Verify:
   - All 5 PNG files exist
   - File names are exactly as listed above
   - Files are in `assets/textures/` folder
   - Folder exists in same directory as .exe

## Example File Check

Run this in PowerShell from build directory:
```powershell
Test-Path ..\assets\textures\aircraft_carrier.png
Test-Path ..\assets\textures\battleship.png
Test-Path ..\assets\textures\cruiser.png
Test-Path ..\assets\textures\submarine.png
Test-Path ..\assets\textures\destroyer.png
```

All should return `True`.

---

**Happy sailing with your custom ships! ⚓🎨**
