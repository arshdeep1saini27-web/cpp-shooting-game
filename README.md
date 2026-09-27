# Shadow Strike

A realistic-feeling C++ shooter prototype built with SFML.

Features:
- player movement and shooting
- AI enemies that chase and retaliate
- animated character-like shapes and a scope overlay
- loading screen
- generated sound effects and looping ambient music
- mod configuration through a JSON file
- open/close scope with Tab key
- simple HUD and wave-based survival gameplay

Build instructions:

1. Install SFML 2.5+.
2. Configure the project:
   cmake -S . -B build
3. Build:
   cmake --build build
4. Run:
   ./build/shadowstrike

Controls:
- WASD / Arrow keys: move
- Mouse: aim
- Left mouse: shoot
- Tab: open/close scope
- Esc: quit

Game design notes:
- The project uses generated procedural audio instead of requiring external media files.
- The mod system reads values from mods/default_mod.json.
- You can tune difficulty and feel by editing that JSON file.
