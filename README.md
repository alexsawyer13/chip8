# Chip 8 emulator by Alex Sawyer

## Introduction
Inspired by blog by Tobias V. Langhoff

https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

External roms can be found below
- https://johnearnest.github.io/chip8Archive/
- https://github.com/corax89/chip8-test-rom

## Instructions

Cmake is used to build the project

The preferred way to build the project is running scripts/release.py from the chip8 directory

This will create a release directory. The chip8 directory inside contains all files necessary for the program to run

Alternatively Cmake can be used directly:

Run from the chip8 directory to create build files
- cmake -S . -B out

Run from the chip8 directory to build the executable
- cmake --build out --config Release

The executable can be found in out/Release

The executable requires SDL2.dll to be in the same directory as it to run

Some other convenience scripts are placed in the scripts directory, read the scripts/README.md for information on how to use them

## Todo
Assembler / disassembler

Chip-48 extension

Implement more platforms

Allow save/load states

Add some more debug features using keyboard
- Speed up tickrate
- Slow down tickrate
- Pause emulation
- Print debug text on screen using SDL
- Maybe make screen bigger and contain chip-8 within a subsection of it to include this
- Maybe use imgui