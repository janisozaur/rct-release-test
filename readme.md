# OpenRCT2
An open source clone of RollerCoaster Tycoon 2 built by decompiling the original game one bit at a time.
- [Screenshot 1, cheat windows](http://imgur.com/a/zJTYd)
- [Screenshot 2, large land tool area](http://i.imgur.com/kTkK5Gw.png) (up to 64 squares at once)
- [Screenshot 3, high resolution](http://i.imgur.com/yFzNyVu.jpg)
- [Screenshot 4, resizable window](http://imgur.com/a/3GDuT)

[![Travis CI](https://travis-ci.org/OpenRCT2/OpenRCT2.svg)](https://travis-ci.org/OpenRCT2/OpenRCT2) [![AppVeyor](https://ci.appveyor.com/api/projects/status/fib6re830brysuo2?svg=true)](https://ci.appveyor.com/project/IntelOrca/openrct2)

[![Gitter](https://badges.gitter.im/Join Chat.svg)](https://gitter.im/OpenRCT2/OpenRCT2?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# Contents
- 1 - [Introduction](#1-introduction)
  - 1.1 - [Background](#11-background)
  - 1.2 - [Decompiling the game](#12-decompiling-the-game)
  - 1.3 - [Progress](#13-progress)
  - 1.4 - [Aim](#14-aim)
- 2 - [Downloading the game / Building the source code](#2-downloading-the-game--building-the-source-code)
  - 2.1 - [Building Prerequisites](#21-building-prerequisites)
  - 2.2 - [Compiling and running](#22-compiling-and-running)
- 3 - [Contributing](#3-contributing)
  - 3.1 - [Decompiling](#31-decompiling)
  - 3.2 - [Naming of procedures and variables](#32-naming-of-procedures-and-variables)
  - 3.3 - [Cleaning and documenting the source code](#33-cleaning-and-documenting-the-source-code)
  - 3.4 - [Implementing new features / fixing bugs](#34-implementing-new-features--fixing-bugs)
  - 3.5 - [Translation](#35-translation)
- 4 - [Licence](#4-licence)

# 1 Introduction

## 1.1 Background
**OpenRCT2** is an attempt to decompile RollerCoaster Tycoon 2 into C. RollerCoaster Tycoon 2 was originally written in MASM and Visual C++ where functions related to interfacing with the operating system were written in C (supposedly 1%), with the rest of the game being written in pure x86 assembly. For an example of this method, OpenTTD was formed through a similar procedure; the original game, Transport Tycoon Deluxe, was decompiled into C which allowed for the addition of thousands of features to the game. RollerCoaster Tycoon 2 uses the third version of Chris Sawyer's engine, which shares some code with Transport Tycoon. This is reflected in the usage of OpenTTD 0.1 code such as the windowing system and graphics rendering. While the version of the engine used in Chris Sawyer's Locomotion is newer, OpenRCT2 is currently targeting the RollerCoaster Tycoon 2 engine to ease the decompilation process.

## 1.2 Decompiling the game
In order to decompile the game gradually without introducing new bugs, each procedure in RollerCoaster Tycoon 2 is to be re-written in C on an individual basis. To test the accuracy of the re-written procedures, the decompiled C procedures are compiled into a DLL (*openrct2.dll*) which exports an entry procedure mimicking the WinMain function in RollerCoaster Tycoon 2. The original executable *rct2.exe* has been patched so that *openrct2.dll* and WinMain are in the DLL import table and the WinMain export procedure in *openrct2.dll* is called at the start of the WinMain procedure in *rct2.exe* before returning. With this system implemented, starting rct2.exe calls the new DLL as part of its initialization; the DLL can then run all the decompiled code whilst still being able to read / write to the *rct2.exe* memory model and run *rct2.exe* procedures.

The project therefore acts as a patch to RollerCoaster Tycoon 2, allowing each procedure to be gradually implemented while simultaneously adding new features where possible. Until all procedures of the original game are re-written in C, the project must remain a DLL which is called from the patched *rct2.exe*.

## 1.3 Progress
Currently, the windowing system, graphics rendering and basic game loop are being decompiled. Decompiling all of the game's procedures is a convenient way of identifying the game's memory structure. SDL2 has been used as a replacement for the operating system calls, allowing for cross-platform support after the dependency on the original game's executable has been removed.

As of 9th September 2015, a vast set of improvements have been added such as improved UI, tools, cheats, localisation and configuration. The entire window system and UI has been implemented, all file format handling such as loading and saving of parks, graphics and tracks, ride logic, game management (e.g. peep generation, awards, cash out) and a lot of the peep logic. What remains is a lot of code to draw the map, the rest of the peep logic, and the entirety of vehicle logic. The rest are small parts around different areas that aren't particularly significant. More information can be found in [changes to original game](https://github.com/OpenRCT2/OpenRCT2/wiki/Changes-to-original-game).

## 1.4 Aim
The aim is to completely decompile RollerCoaster Tycoon 2 into C so that cross-platform support, new features, and new gameplay can be added in a similar fashion to OpenTTD. With the addition of SDL2, the game can already be run in a resizeable window (which was not possible originally). Once the game has been fully decompiled, additional gameplay features, gameplay tweaks, and improvements can be introduced. The following is only a brief, non-exhaustive list of the possibilities - there are many more:

- Improved peep path-finding
- Increased window / ride / object / map / construction limits
- More sandbox-friendly gameplay
- Editing available objects
- Improved title sequence
- Translation into more languages
- Re-introduction of RollerCoaster Tycoon 1 mechanics:
	- Shuttle Loop compatibility
	- Have Fun! objective
	- Finish building five coasters objective
	- Using the mountain tool during the game

# 2 Downloading the game / Building the source code

Several third party build servers offer precompiled builds and installers of the latest stable and the develop branch.
[OpenRCT2.com](https://openrct2.com/download)
[OpenRCT.net](https://openrct.net/builds.php)
[UrsaLabs](https://openrct.ursalabs.co)

There is also a Launcher available from [OpenRCT.net](https://openrct.net/download) that will automatically update your build so that you always have the current version as they are released.

## 2.1 Building prerequisites

OpenRCT2 requires original files of Rollercoaster Tycoon 2 to play. It can be bought at either [Steam](http://store.steampowered.com/app/285330/) or [GOG.com](http://www.gog.com/game/rollercoaster_tycoon_2).

### Windows:
- Windows XP / Vista / 7 / 8 / 10
- Visual Studio 2015 (Enterprise / Professional / [Community (Free)](https://www.visualstudio.com/products/visual-studio-community-vs))
- [Powershell 4.0](http://social.technet.microsoft.com/wiki/contents/articles/21016.how-to-install-windows-powershell-4-0.aspx).

### Mac OS X:
- [Homebrew](http://brew.sh) or [MacPorts](https://www.macports.org/)

### Mac OS X / Linux:
- [MinGW-w64](http://mingw-w64.sourceforge.net/)
- [Wine](http://www.winehq.org)
- libsdl2 compiled with MinGW-w64

## 2.2 Compiling and running
### Windows:
1. Check out the repository. This can be done using [GitHub Windows](https://windows.github.com/) or [other tools](https://help.github.com/articles/which-remote-url-should-i-use).
2. Open the solution in the projects directory (**openrct2.sln**) with Visual Studio.
3. [Select the 'Release' configuration](http://msdn.microsoft.com/en-us/library/wx0123s5.aspx) and click Build -> Rebuild Solution. The dropdown menu to enable the 'release' configuration is towards the top of the VS Express window, near the "TEST" menu.
4. Start debugging. Press the "Local Windows Debugger" button with a green "play" icon next to it. If Visual Studio shows a warning about *openrct2.exe* not having debug information, press Continue.
5. When OpenRCT2 is run for the first time, it creates a settings file in `My Documents/OpenRCT2`. If it can't find the original installation of RCT2, you will need to edit `config.ini` in that folder and change the value of `game_path` to where RCT2 is installed.
6. If the game crashes, you may need to press the red, square Stop button along the top of VS Express to stop the program.

Alternatively, to simply build the project you can just execute ``build.bat`` within VS2015 Native Tools Command prompt or Github Windows prompt.

### Mac OS X:
Providing Homebrew or MacPorts are installed (but not both!), OpenRCT2's dependencies and Wine can be installed automatically through `install.sh`.
```
bash install.sh
bash build.sh
wine openrct2.exe
```

### Linux:
As the easiest approach depends on your distribution, please take a look at the [wiki](https://github.com/OpenRCT2/OpenRCT2/wiki).


# 3 Contributing
OpenRCT2 uses the [gitflow workflow](https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow). If you are implementing a new feature or logic from the original game, please branch off and perform pull requests to **develop**. If you are fixing a bug for the next release, please branch off and perform pull requests to the correct release branch. **master** only contains tagged releases, you should never branch off this.

Please read our [contributing guidelines](https://github.com/OpenRCT2/OpenRCT2/blob/develop/CONTRIBUTING.md) for information.

## 3.1 Decompiling
Experience with reverse engineering and x86 assembly is necessary to decompile the original game. [IDA 5.0](https://www.hex-rays.com/products/ida/support/download_freeware.shtml) is currently being used to disassemble and analyze the game. You are welcome to contribute to the process by taking an undecompiled procedure, disassembling it, and rewriting it in C. For more information and the latest IDA database, contact IntelOrca.

## 3.2 Naming of procedures and variables
During the development phase, many variables and procedures are referenced by their memory address in OpenRCT2. This is a result of ongoing reverse engineering efforts; the functionality and use of these values has yet to be determined. To help with identification, there are multiple methods you can use.
For variables:
- Modify the variable and see how the game is affected
- Reverse-engineer RCT2 and find where the variable is used or modified

For procedures:
- Remove the call to the procedure in OpenRCT2 and observe the effects
- Reverse-engineer RCT2 and find where the procedure is called

## 3.3 Cleaning and documenting the source code
As the source code is formed from decompilation, it is unorganized and undocumented. Efforts towards cleaning up and documenting code are appreciated; for example, blocks of code can be moved into their own functions, and macros can be created for operations that occur frequently. However, be aware that the overall structure of the code should remain the same to ensure that OpenRCT2 is kept in sync with the original game to ease the integration of additional decompiled code.

In general, small changes that improve code quality and make it easier to reason about the logic are appreciated. More significant changes, such as changing the game's architecture, are to be avoided during the ongoing decompilation of the original game.

## 3.4 Implementing new features / fixing bugs
While decompilation is an ongoing process, this does not prohibit changes being made to the game. New features or bugfixes can be added, with caution, if the underlying code has been decompiled. When implementing these changes, ensure that comments are added to clearly identify where code has been intentionally changed so that it functions differently to the original game; this is essential to ensuring all research from reverse-engineering can still be applied.

## 3.5 Translation
Translations are in progress for German, Dutch, French, Hungarian, Polish, Spanish, Swedish, Italian, and more. You can translate the game into other languages by editing the language files in the data directory. Please join discussions and submit pull requests to https://github.com/OpenRCT2/Localisation.

# 4 Licence
**OpenRCT2** is licensed under the GNU General Public License version 3.

# 5 More information
- [GitHub](https://github.com/OpenRCT2/OpenRCT2)
- [Facebook](https://www.facebook.com/OpenRCT2)
- [OpenRCT2.com](https://openrct2.com)
- [OpenRCT.net](https://openrct.net)
- [rct2 subreddit](http://www.reddit.com/r/rct/)
- [openrct2 subreddit](http://www.reddit.com/r/openrct2)
- [openrct2 subverse](http://www.voat.co/v/openrct2)
