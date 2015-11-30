# Geometry-Splitter-Maya-Plugin

## Requirements
* Maya (+ DevKit for Maya 2016)
* CMake

## Generate build files
Create a build-folder and run CMake for your platform. Set -DMAYA_VERSION to your Maya version (default is 2016).

Example: How to generate build files for windows using Visual Studio 2012 compiler and Maya 2016:
```bash
cmake -G "Visual Studio 11 2012 Win64" -DMAYA_VERSION=2016 ../
```

## Build with CMake
You can use the same build command for all platforms using CMake.

```bash
cmake --build . --config Release --target Install
```

The compiled plugin will be stored in the 'install' folder.
