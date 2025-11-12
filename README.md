# ColorsSRE v1.1.0
------------------

This is the 1.1.0 version of Colors Software Rendering Engine, written in C using **SDL3**.

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

- Rendering a .obj mesh object file with a .png image as texture
    
- A controllable light source object with Blinn-Phong shading
  
- Controllable camera and object positions

## v1.1.0 Update

- Key binding update
  
## Keyboard bindings

WASD = Move camera

IJKL + UH = Move light source (H moves light source backwards, U moves light source forward

Arrow Keys (Left, Right, Up, Down) = Rotate object

B = Reset camera to origin, facing forward (use if mouse moves too far and object is lost)

M = Freeze mouse input (toggle on/off)

ESC = Quit application

# Rendering with custom meshes and textures

To render custom meshes and textures, run the application from the command line:

`ColorsSRE.exe <path_to_obj_file> <path_to_png_file>`

- The .obj file must be in the second argument.

- The .png file must be in the third argument.

  Note: it is recommended to place the .obj and .png files in a the 'assets' folder and access it with `../assets/file_name.objorpng`

- If the program fails to open the files, it will use the default assets instead.

- Any additional arguments are ignored.

## Previous Versions Update Descriptions

### v1.0.0

- Added support for rendering custom .obj meshes and .png files
  
- Convert from Win32 to SDL3 for displaying and window management
