\# BROK CS2 Internal



Internal Cheat for Counter-Strike 2.



\## Features

\- ESP (Box, Health, Snaplines)

\- Aimbot (Custom FOV, Smooth, Hitbox selection)

\- Silent Aim (ViewAngles override)

\- Bunnyhop (Auto-jump)

\- Custom ImGui Menu (Mouse 3 to open)



\## Build Instructions

To build the DLL, use `w64devkit` (MinGW):

```bash

g++ -shared -O2 -DUNICODE -D\_UNICODE -o cs2\_internal.dll main.cpp config.cpp visuals.cpp aimbot.cpp silent.cpp movement.cpp menu.cpp imgui.cpp imgui\_draw.cpp imgui\_tables.cpp imgui\_widgets.cpp imgui\_impl\_win32.cpp imgui\_impl\_dx11.cpp -lgdi32 -luser32 -ld3d11 -ldxgi -ld3dcompiler -ldxguid -lole32 -luuid -limm32 -ldwmapi -lpsapi -static -static-libgcc -static-libstdc++

