::@echo off
:: > Setup required Environment
:: -------------------------------------
set COMPILER_DIR=C:\\raylib\\w64devkit\\bin
set PATH=%PATH%;%COMPILER_DIR%
cd %~dp0
:: .
:: > Compile simple .rc file
:: ----------------------------
cd ..\..\src
cmd /c windres cool_project.rc -o cool_project.rc.data
:: .
:: > Generating project
:: --------------------------
cmd /c mingw32-make -f Makefile ^
PROJECT_NAME=cool_project ^
PROJECT_VERSION=1.0 ^
PROJECT_DESCRIPTION="A very cool project game" ^
PROJECT_INTERNAL_NAME=cool_project ^
PROJECT_PLATFORM=PLATFORM_DESKTOP ^
PROJECT_SOURCE_FILES="cool_project.c" ^
BUILD_MODE="RELEASE"
:: > Return to scripts directory
:: -----------------------------
cd ..\projects\scripts