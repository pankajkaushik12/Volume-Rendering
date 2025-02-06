# Volume Rendering with OpenGL (C++)

This project demonstrates volume rendering using OpenGL and C++. The goal is to visualize 3D volumetric data in real-time by using advanced OpenGL shaders and rendering techniques.

## Features
- 3D volume rendering using raycasting.
- Interactive visualization (rotate, zoom, pan).
- Support for multiple volume data formats (e.g., `.raw`, `.nii`).
- Shader-based rendering pipeline for efficient computation.
- OpenGL and C++ performance optimizations for real-time display.

## Prerequisites
- C++ compiler
- OpenGL (version 4.3+ recommended)
- GLFW for window/context management
- GLEW (or another OpenGL extension loader)
- CMake for build configuration

## Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/pankajkaushik12/Volume-Rendering.git
   cd Volume-Rendering
2. Install the dependencies (OpenGL, GLFW, GLEW)
3. Set the path to GLEW and GLFW as environment variable in *GLEW_DIR* & *GLFW_DIR*.
4. Use the CMake to build and generate the solution. Open the generated solution in Microsoft Visual Studio.
4. Run the project
    ```bash
    VolumeRendering.exe -volumePath *volume-to-volume*

## Controls:
- Left-click and drag to rotate the volume.
- Press 'Esc' to exit the application.

## 1D Transfer Function
The 1D transfer function is used to map voxel intensities to color and opacity values for volume rendering. By default, a linear transfer function is applied. You can customize the transfer function using the following controls:
- Left-click on the palette to add a new control point.
- Right-click on a control point to select it.
- Drag the opacity slider to adjust the opacity of the selected control point.
- Click the cross button to delete a selected control point.
- Drag the position slider to adjust the intensity corresponding to the control point.

