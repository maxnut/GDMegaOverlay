# GDmenu

Join the discord: [link](https://discord.gg/nbDjEg7SSU)

## How to use

Download it from releases (or compile it) and extract all files inside the GD folder.

Do not download the source code to install the menu

![alt text](https://media.discordapp.net/attachments/587598582308143125/1055961835888722010/20221223223845_1.jpg)

## Development

<span style="color:red">THIS PROJECT CAN BE COMPILED ONLY ON WINDOWS</span>

Some dependencies are needed:
* [Git](https://git-scm.com/)
* [Visual Studio](https://visualstudio.microsoft.com/)
* [Visual Studio Code](https://code.visualstudio.com/)
* [Cmake](https://cmake.org/)

Now you can directly clone this repo or you can fork it.

Run this in your terminal on the forlder where you want all the code:

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
cmake --build build --config Release -j 4
```

### Setting up the dev enviroment with VSCode

Open GDmenu folder with Visual Studio Code.

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