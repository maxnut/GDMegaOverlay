# GDmenu

Free, feature-rich, gui-integrated geometry dash mod menu

![alt text](https://media.discordapp.net/attachments/587598582308143125/1055961835888722010/20221223223845_1.jpg)

## Features

- basic hacks
- startpos switcher
- show hitboxes
- internal recorder
- macro bot
- click bot
- and much more!

Any suggestions or bug reports?
Join the discord server [here](https://discord.gg/nbDjEg7SSU)

## How to use

Download the latest [release](https://github.com/maxnut/GDmenu/releases/latest) and extract all files inside the GD folder.

## How to build
Make sure you have [git](https://git-scm.com/downloads) and [cmake](https://cmake.org/download/) installed and in your PATH

1. Clone
```bash
git clone https://github.com/maxnut/GDmenu --recursive
cd GDmenu
```
2. Configure
``` bash
cmake -B build -A win32
```
3. Build
```bash
# do not use Debug
cmake --build build --config Release
```
