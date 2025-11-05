# Quick Start: Adding Your Ship Sprites

## Step 1: Prepare Your Images

You have pixel art ship images in PNG format. Perfect!

### Required Files

Rename/copy your images to match these exact names:
- `aircraft_carrier.png` - Your longest ship (5 cells)
- `battleship.png` - Second longest (4 cells)
- `cruiser.png` - Medium ship (3 cells)
- `submarine.png` - Medium ship (3 cells)
- `destroyer.png` - Smallest ship (2 cells)

### Tips
- **Orientation:** Ships should face RIGHT →
- **Format:** PNG with transparent background
- **Size:** Any size works, but ~40 pixels high is ideal
- **Length:** Width should be proportional to ship size (5:1, 4:1, 3:1, etc.)

## Step 2: Copy to Assets Folder

```powershell
# From your project root
cd C:\Users\eshwa\Repos\battleship

# Copy your images
copy path\to\your\aircraft_carrier_image.png assets\textures\aircraft_carrier.png
copy path\to\your\battleship_image.png assets\textures\battleship.png
copy path\to\your\cruiser_image.png assets\textures\cruiser.png
copy path\to\your\submarine_image.png assets\textures\submarine.png
copy path\to\your\destroyer_image.png assets\textures\destroyer.png
```

## Step 3: Run the Game

```powershell
cd build
.\fleet_commander_gui.exe
```

## What to Expect

### If All Images Load Successfully:
```
Loaded texture: assets/textures/aircraft_carrier.png
Loaded texture: assets/textures/battleship.png
Loaded texture: assets/textures/cruiser.png
Loaded texture: assets/textures/submarine.png
Loaded texture: assets/textures/destroyer.png
Ship sprites enabled!
```

You'll see your custom ship images in the game! 🎨

### If Some Images Fail:
```
Warning: Could not load assets/textures/battleship.png
Using default colored rectangles for ships.
```

Game falls back to simple colored rectangles. Check:
- File names match exactly
- Files are in correct folder
- Files are valid PNG format

## Sprite Features

When sprites are active:
- ✓ Your custom images display on the board
- ✓ Ships automatically rotate for vertical placement
- ✓ Hit markers (red circles) appear on hits
- ✓ Sunk ships become gray and semi-transparent
- ✓ Works on both your board and enemy board

## Example Command Sequence

From project root:
```powershell
# 1. Place your images
copy my_ships\*.png assets\textures\

# 2. Rename if needed
cd assets\textures
ren my_carrier.png aircraft_carrier.png
ren my_battleship.png battleship.png
# etc...

# 3. Run game
cd ..\..\build
.\fleet_commander_gui.exe
```

## Still Need Help?

See `SHIP_SPRITES_GUIDE.md` for:
- Detailed image requirements
- Creating pixel art from scratch
- Troubleshooting sprite issues
- Advanced customization options

---

**Your pixel art ships are ready to sail! ⚓🎨**
