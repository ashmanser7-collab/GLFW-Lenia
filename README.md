# Lenia (cellular automaton) — `lenia.cpp`

This folder contains an implementation of Lenia-like continuous cellular automata (CA) using immediate-mode OpenGL for visualization. The program computes a kernel and growth function and iterates the CA on a grid. It supports mouse drawing and pausing to edit parameters.

Highlights
- Grid size: 256x256 (controlled by `size`) with a tunable radius and kernel.
- Parameters at the top of `lenia.cpp`: `kernel_coeficient`, `growth_coeficient`, `mu`, `dt`, `radius`, `resolution`.
- Pause ('P') enters an interactive mode where you can draw with the mouse and then resume.

Build & run
- Requires GLFW and OpenGL. Link with -lglfw -lGL or -lglfw3 -lopengl32 on Windows.

Example (Windows MSYS2 / mingw):

```powershell
g++ "lenia.cpp" -o "lenia.exe" -lglfw3 -lopengl32 -lgdi32 -std=c++17 -pthread
.\\lenia.exe
```

Controls
- P — pause / enter editing mode (then left-click to draw patterns). Press P again to resume.

Notes
- The code contains presets (`splotch`, `randomSplotches`) and utilities for computing the kernel and growth table. Look at `computeKernel`, `computeGrowth`, and `threadStepAll` for the core logic.
- Rendering uses GL_POINTS and `glPointSize(resolution)`.

Suggested improvements
- Add a small GUI for live parameter tweaking (Dear ImGui) and saving/loading states.
- Export the kernel/growth parameters as presets and expose them through command-line flags.
