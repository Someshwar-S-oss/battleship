GAME TEXTURES
=============

SHIP SPRITES (Required for custom ship graphics)
-------------------------------------------------
Place your 5 ship PNG files here with these exact names:

- aircraft_carrier.png  (5 cells long)
- battleship.png        (4 cells long)
- cruiser.png           (3 cells long)
- submarine.png         (3 cells long)
- destroyer.png         (2 cells long)

Ship Image Requirements:
- Format: PNG with transparency
- Orientation: Horizontal (game will auto-rotate for vertical ships)
- Current cell size: 60 pixels
- Recommended dimensions:
  * Aircraft Carrier: 300×60 px (or 600×120 for 2x)
  * Battleship: 240×60 px (or 480×120 for 2x)
  * Cruiser: 180×60 px (or 360×120 for 2x)
  * Submarine: 180×60 px (or 360×120 for 2x)
  * Destroyer: 120×60 px (or 240×120 for 2x)
- Style: Pixel art for retro look

The game will automatically use these sprites when all 5 files are present.
If any are missing, it falls back to colored rectangles.


MENU BACKGROUND (Optional)
---------------------------
- menu_background.png

Background Image Requirements:
- Format: PNG (can have transparency or be opaque)
- Recommended size: 1920×1080 pixels (will be scaled to fit)
- Style: Ocean/naval themed recommended
- The game will automatically scale it to fit the window

If this file is not present, the game will use the default dark background.
