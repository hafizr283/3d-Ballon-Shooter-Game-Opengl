# 3D Balloon Shooter

A first-person 3D balloon-defense game built in C++ with modern OpenGL. The player looks around from a tower, shoots falling balloons, and tries to prevent any balloon from reaching the ground.

## Output Video

[![Watch the gameplay output video](https://drive.google.com/thumbnail?id=14Tgp8xRURTGbS4DHvnSRGTu9-ikXec_T&sz=w1200)](https://drive.google.com/file/d/14Tgp8xRURTGbS4DHvnSRGTu9-ikXec_T/view?usp=sharing)

[Open the gameplay output video on Google Drive](https://drive.google.com/file/d/14Tgp8xRURTGbS4DHvnSRGTu9-ikXec_T/view?usp=sharing)

## Suggested repository name

`opengl-3d-balloon-shooter`

Other good choices are `3d-balloon-defense` and `cpp-opengl-balloon-shooter`. The first name is the clearest because it identifies both the technology and the game.

## What is implemented

- First-person camera with mouse look and WASD movement, plus vertical movement.
- Falling balloon spawning with randomized positions and speeds.
- Balloon collision detection and pop animation.
- Score HUD and game-over screen when a balloon reaches the ground.
- Restartable game state (`R` after game over).
- Two firing modes: a limited-ammo bullet gun and an arrow launcher.
- Ammo counters, cooldowns, reload timers, recoil, and projectile movement.
- Day/night environment switching.
- Directional, point, and spot lights, including tower and tree lamps.
- Independent ambient, diffuse, and specular lighting toggles.
- Phong and Gouraud shading comparison.
- Texture on/off mode and procedural balloon shading option.
- Rain mode with wind, faster balloon falling, rain particles, and occasional thunder flashes.
- Four-way split viewport for rendering comparison.
- Automatic scenario tour for demonstrating the scene and graphics features.
- A modeled environment containing a tower, road, buildings/tiles, trees, lamps, lighthouse, water/sky, vehicles, and other scene objects.
- Texture assets stored in the `image/` directory and GLSL shaders stored in the project root.

## Technology

- C++20
- OpenGL 3.3 Core Profile
- GLFW for window creation and input
- GLAD for loading OpenGL functions
- GLM for vectors, matrices, camera math, and transformations
- stb_image for image loading
- GLSL vertex and fragment shaders

## Repository layout

```text
main.cpp                         Game loop, scene setup, gameplay, input, rendering
camera.h / basic_camera.h         Camera and FPS-style movement
shader.h                          Shader compilation and uniform helpers
sphere.h / cone.h                 Procedural geometry helpers
*Light.h                          Directional, point, and spot light classes
vertexShader*.vs                  Vertex shaders
fragmentShader*.fs                Fragment shaders
image/                            Environment and material textures
TowerDefenseBalloonShooter.vcxproj Visual Studio project
x64/Debug/                        Local build output (do not commit generated files)
```

## Controls

### Gameplay and camera

| Key/input | Action |
| --- | --- |
| `W A S D` | Move the camera |
| `Q` / `E` | Move up / down |
| Mouse | Look around |
| `Space` | Fire the current weapon |
| Right mouse button | Fire an arrow when Arrow mode is active |
| `Ctrl+M` | Switch Bullet / Arrow mode |
| `R` | Reload arrows while playing; restart after game over |
| `+` / `-` | Increase / decrease camera speed |
| `Esc` | Quit |

### Graphics and simulation

| Key | Action |
| --- | --- |
| `M` | Release/capture the mouse cursor |
| `N` | Toggle day/night |
| `1` | Toggle sun/moon directional light |
| `2` | Toggle tower point light |
| `K` | Toggle tree point light |
| `3` | Toggle flashlight spot light |
| `4` | Freeze/unfreeze balloons |
| `5` / `6` / `7` | Toggle ambient / diffuse / specular components |
| `8` | Toggle Phong/Gouraud shading |
| `9` or `T` | Toggle textures |
| `U` | Toggle procedural balloon gradient shading |
| `F2` | Toggle rain and weather physics |
| `V` | Toggle split viewport |
| `0` | Start/stop the automatic scenario tour |
| `Y` | Toggle sky texture/vertex-color presentation |

Some legacy transformation/demo keys (`I`, `J`, `L`, `O`, `P`, `C`, `X`, `Z`, `H`, `F`, `G`) are also present in `main.cpp` for graphics-laboratory demonstrations.

## How the game works

At startup, the scene creates a tower environment and two initial balloons. During the game, new balloons are spawned approximately every two seconds above the scene. Active balloons fall toward the ground; rain increases their fall speed and applies wind drift. Shooting a balloon marks it as popping, plays a short scale-up animation, removes it, and increments the score. If any active balloon reaches the ground, the game stops and displays the final score. Press `R` to reset the score, ammunition, projectiles, and initial balloons.

The renderer can switch between Phong and Gouraud shading and can independently enable/disable textures and lighting components. These modes are intended both for gameplay presentation and for comparing computer-graphics techniques.

## Building in Visual Studio (Windows)

1. Install **Visual Studio 2022** (or a compatible version) with **Desktop development with C++** and the Windows 10/11 SDK.
2. Install or obtain the required native libraries:
   - GLFW (library file: `glfw3.lib`)
   - GLAD (`glad.c` and the `glad/` + `KHR/` headers)
   - GLM headers
3. Open `TowerDefenseBalloonShooter.vcxproj` in Visual Studio.
4. Select `Debug` or `Release` and `x64` in the toolbar.
5. Verify the include/library paths. The checked-in project currently expects an OpenGL dependency installation at:

   ```text
   H:\opengl\include
   H:\opengl\lib
   ```

   These paths are machine-specific. For another computer, edit the project/property sheet or configure equivalent paths in **Project Properties → C/C++ → Additional Include Directories** and **Linker → Additional Library Directories**. The project also references GLAD through a relative path (`..\..\..\src\glad.c`), so update that source path if GLAD is stored elsewhere.

6. Build with **Build → Build Solution**.
7. Run with **Debug → Start Without Debugging** (`Ctrl+F5`). Keep the working directory at the project directory so shader files and `image/` textures can be found.

The generated executable is normally placed under `x64\\Debug\\` or `x64\\Release\\`.

## Common build/runtime issues

- **Cannot open `glfw3.lib`:** install GLFW and correct the library directory.
- **Cannot open `glad/glad.h` or `glad.c`:** add the GLAD include directory and correct the source path in the project.
- **Cannot open GLM headers:** add the directory containing the `glm` folder to the include path.
- **Shaders/textures not found:** run from the repository/project directory; the program loads shader filenames and `image/...` using relative paths.
- **Black or incomplete scene:** confirm that an OpenGL 3.3-capable graphics driver is installed.

## Git recommendations

Commit source, shaders, headers, project files, and the required `image/` assets. Do not commit generated Visual Studio output (`x64/`, `.vs/`, `.user` files), temporary logs, or the ZIP archives unless they are intentionally being used as release artifacts. A small `.gitignore` for Visual Studio build output should be added before the first push.

## Academic context

The project includes `assignment.tex`, presentation material, and screenshots related to a computer-graphics laboratory assignment. The game itself is implemented in `main.cpp` and uses the accompanying shader and geometry helper files.

## License

No license is currently declared. Add a license file (for example MIT) before publishing if you want others to reuse or modify the project.
