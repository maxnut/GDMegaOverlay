<div align="center">
  
# GD Mega Overlay

[![Discord](https://img.shields.io/badge/Discord-white?style=flat&logo=discord)](https://discord.gg/nbDjEg7SSU)
</div>

## Description

Free, feature-rich, gui-integrated geometry dash mod menu

![alt text](https://media.discordapp.net/attachments/587598582308143125/1064198417804431491/20230115160755_1.jpg)

## Features

* Basic hacks
* Startpos switcher
* Show hitboxes
* Internal recorder
* Macro bot
* Click bot
* And much more!

## How to use

Download the latest [release](https://github.com/maxnut/GDMegaOverlay/releases/latest) and extract all files inside the GD folder.

Additional steps to run on Linux: Go to `Steam Library` -> Right click Geometry Dash -> `Properties` -> `General` -> `Launch Options` and add `WINEDLLOVERRIDES="xinput9_1_0=n,b" %command%`

## Development

<span style="color:red">THIS PROJECT CAN BE COMPILED ONLY ON WINDOWS</span>

Some dependencies are needed:
* [Git](https://git-scm.com/)
* [Visual Studio](https://visualstudio.microsoft.com/)
* [CMake](https://cmake.org/)

Now you can directly clone this repo or you can fork it.

Run this in your terminal on the folder where you want all the code:

1. Clone
```bash
git clone https://github.com/maxnut/GDMegaOverlay --recursive
cd GDMegaOverlay
```

2. Configure
```bash
cmake -B build -A win32
```

3. Build

```bash
# do not use Debug
cmake --build build --config Release -j 4
```

### Setting up the dev enviroment with VSCode (optional)

Open GDMegaOverlay folder with Visual Studio Code.

Now VSCode will prompt you to install some recommended extensions, accept or else you'll need to install them manually.
The extensions are:
* CMake by twxs
* Cmake Tools by Microsoft

After extensions installation, you need to configure the project:
* The compiler should be [Visual Studio Community 2022 Release - x86]
* Press the play (configure) button at the middle-bottom of VSCode
* Press the Build button that should be near the play one 
![alt text](/docs/img/vscode_config.png)

You should find the result in build/Release

## Credits

[altalk23/HitboxVisualizer](https://github.com/altalk23/HitboxVisualizer) for the hitbox code (i converted it from geode to gd.h, corrected a bug and implemented them in the editor)

[matcool/ReplayBot](https://github.com/matcool/ReplayBot) internal recorder, i integrated my clickbot into it

[Very-Tall-Midget/OmegaBot2](https://github.com/Very-Tall-Midget/OmegaBot2) practice fixes

[adafcaefc/ProxyDllLoader](https://github.com/adafcaefc/ProxyDllLoader) for the mod loader

[TpdEA](https://discord.gg/AXhTdHUd2R) for the json files and the endscreen code

[Absolute](https://github.com/absoIute) for the json files, the gd::string and the demonlist/challenge list files

[yt-dlp](https://github.com/yt-dlp/yt-dlp) for youtube audio downloading
