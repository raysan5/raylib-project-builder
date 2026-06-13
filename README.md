# `rpb` - raylib project builder

![raylib project builder](screenshots/rpb_v010_shot01.png)

### [USE `rpb` TOOL ONLINE!](https://raysan5.itch.io/raylib-project-builder)

## What can I do with `rpb`?

Welcome to `rpb`! A new tool to help you build your raylib projects, created with raylib project creator!

## Features

 - Support `.rpc` **project configuration** interchange file format
    - Shared with homonymous tool: [`rpc` - raylib project creator](https://github.com/raysan5/raylib-project-creator)
 - Multiple platforms building supported: Windows, Linux, Web
 - Configure project build settings in a visual way
 - Automatic assets validation system 
 - Command-line support for **automated project building**
 - **Completely portable (single-file, no-dependencies)**

## Basic Usage

Open the tool, drag & drop your `.rpc` project file. Configure project build properties...

`rpb` dektop version comes with command-line support for automated project building. You can check all available options with the following command:

 > rpb.exe --help

### Platforms Supported

`rpb` can build for multiple platforms...

...

## Expected project structure

The expected project structure defined by the `.rpc` file should follow the template structure and contrains the following elements:

```
project-repo-name/
├── src/                // Project source code files
│   ├── resources       // Project assets
│   ├── project_name.c
│   └── Makefile        // Project multi-platform Makefile building (default)
├── raylib/             // [OPTIONAL] raylib library sources
├── projects/           // Project build systems, preconfigured
│   ├── VS2022/         // Build system: Visual Studio 2022
│   ├── VSCode/         // Build system: VSCode
│   └── scripts/        // Build system: scripts (.bat, .sh)
├── imagery/            // Project imagery: icons, banners, logo, splash...
├── .github/
│   └── workflows/      // GitHub Actions: Windows, Linux, macOS
├── .gitignore          // Project files to ignore, preconfigured for build systems
├── README.md           // Project main documentation (from template)
├── LICENSE             // Project source code license (MIT by default)
├── EULA.txt            // [OPTIONAL] Project End-User-Agreement
└── project_name.rpc    // Project configuration file, useful for [rpb] tool
```

## Project Configuration file

`rpb` supports loading raylib project configuration files: `.rpc`.

`.rpb` configuration files are shared betten `rpc` and `rpb` tools. `rpc` generates a base configuration file on project generation with provided properties and `rpb` can use that config file information to build the project for multiple platforms.

`.rpc` is a text and open file format, following a `.ini` style, and can be freely edited with any text editor.

## Keyboard/Mouse Shortcuts

 - `F1` - Show Help window
 - `F2` - Show About window
 - `F3` - Show Sponsor window

**File Options**


## Command-line

```
USAGE:
    > rpb [--help] ...

OPTIONS:
    -h, --help                          : Show tool version and command line usage help

EXAMPLES:
    > rpb ....
```

## Technologies

This tool has been created using the following open-source technologies:

 - [raylib](https://github.com/raysan5/raylib) - A simple and easy-to-use library to enjoy videogames programming
 - [raygui](https://github.com/raysan5/raygui) - A simple and easy-to-use immediate-mode-gui library
 - [rini](https://github.com/raysan5/rini) - A simple and easy-to-use config init files reader and writer
 - [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) - File dialogs for desktop platforms

## Handmade Software

`rpb` is handmade software, it has been carefully developed using the C programming language, with great attention put on each line of code written.
This approach usually results in highly optimized and efficient code, but it is also more time-consuming and require a higher level of technical skills.
The result is great performance and small memory footprint.

`rpb` is self-contained in a portable package of about **1 MB**.

## Issues & Feedback

Issues and feedback can be reported at https://github.com/raysan5/raylib-project-builder.

For additional support, **priority issues review or tool customization** requests, please contact `ray[at]raylib.com`

## License

This project sources are licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

*Copyright (c) 2025-2026 Ramon Santamaria ([@raysan5](https://github.com/raysan5))*
