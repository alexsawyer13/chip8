import os
import shutil

os.system("CALL scripts/configure.bat")
os.system("cmake --build out --config Release")

if os.path.isdir("release"):
    shutil.rmtree("release")

os.mkdir("release")
os.mkdir("release/chip8")

shutil.copy("out/Release/c8.exe", "release/chip8/c8.exe")
shutil.copy("out/Release/c8a.exe", "release/chip8/c8a.exe")
shutil.copy("out/Release/SDL2.dll", "release/chip8/SDL2.dll")
shutil.copytree("fonts", "release/chip8/fonts")
shutil.copytree("roms", "release/chip8/roms")