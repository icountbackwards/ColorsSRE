# ColorsSRE v1.0.1
------------------

This is the 1.0.1 version of ColorsSRE. ColorsSRE is a basic software rendering engine written in C using **SDL3**.

## How to Build

```
git clone https://github.com/icountbackwards/ColorsSRE.git
cd ColorsSRE

mkdir build
cd build
cmake ..
cmake --build . --config Release
```

# Supported features

- Rendering single .obj mesh object file
  
- Applying .png image file as texture
  
- A controllable light source object with Blinn-Phong shading
  
- Controllable camera and mouse input

## v1.0.1 Update

- Minor code restructuring

## Keyboard bindings

WASD = Move camera

IJKL = Move light source

    Note: If V is pressed, I and K move the light source vertically.
          Otherwise, I and K move the light source horizontally.
          
Arrow Keys (Left, Right, Up, Down) = Rotate object

B = Reset camera to origin, facing forward

V = Toggle I/K movement direction

M = Freeze mouse input (toggle on/off)

ESC = Quit application

# Rendering with custom meshes and textures

To render custom meshes and textures, run the application from the command line:

`ColorsSRE.exe <path_to_obj_file> <path_to_png_file>`

- The .obj file should be the second argument.

- The .png file should be the third argument.

- If the program fails to open the files, it will use the default assets instead.

- Any additional arguments are ignored.

## Previous Versions Update Descriptions

## v1.0.0

- Added support for rendering custom .obj meshes and .png files
  
- Uses SDL3 to render frame buffer
