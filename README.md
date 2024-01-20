<div align="center">
  
# GD Mega Overlay
[![Discord](https://img.shields.io/badge/Discord-white?style=flat&logo=discord)](https://discord.gg/nbDjEg7SSU)
</div>

# Not updated to 2.203 yet

## How to use

> **Warning**
> Do __not__ download the source code

Download the latest [release](https://github.com/maxnut/GDMegaOverlay/releases/latest) and extract all files inside the GD folder.

## Description

Free, feature-rich, gui-integrated geometry dash mod menu

![Menu screen](/img/screen.jpg)

## Features

* Basic hacks
* Startpos Switcher
* Macro Bot
* and much more!

## Building
Note: Building is only possible on Windows.
CMake is required to build the project.

* Clone the repository
* `cd` into the repository
* Create the build files with `cmake -B build -DCMAKE_BUILD_TYPE=Release -A win32`
* Compile using `cmake --build build --config Release`

This will give you a `GDMO.dll` inside the build folder.

## Credits

[adafcaefc/ProxyDllLoader](https://github.com/adafcaefc/ProxyDllLoader) for the mod loader

[SpaghettDev](https://github.com/SpaghettDev) helped a ton with various features

[TpdEA](https://github.com/TpdeaX) for help with patches
