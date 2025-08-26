# ColorsSRE

This is the Minimum Viable Product version of ColorsSRE.
ColorsSRE is a basic software rendering engine written in C using Win32 API.

# How to Build

```
git clone https://github.com/icountbackwards/ColorsSRE.git
cd ColorsSRE

mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Supported features in this version:
- Rendering .obj mesh object files
- Applying .png image files as textures
- A controllable light source object with Blinn-Phong shading
- Controllable camera and mouse input

Keyboard bindings:

WASD = Move camera

IJKL = Move light source

    Note: If V is pressed, I and K move the light source vertically.
          Otherwise, I and K move the light source horizontally.
          
Arrow Keys (Left, Right, Up, Down) = Rotate object

B = Reset camera to origin, facing forward

V = Toggle I/K movement direction

M = Freeze mouse input (toggle on/off)

ESC = Quit application

Note : This version does not support rendering for custom mesh object and texture files.
 To render custom .obj mesh and .png image files, replace the .obj and .png files in /assets/ to your own .obj and .png files. And then rename the files to 'suzanne_smooth.obj' and 'gold.png'

