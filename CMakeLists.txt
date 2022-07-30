cmake_minimum_required(VERSION 3.7)

project(chip8)

add_subdirectory(deps/SDL)

# Emulator

add_executable(c8
    src/emulator.c
    src/common/chip8.h
    src/common/chip8.c
    src/common/instructions.h
    src/common/instructions.c
    src/common/graphics.h
    src/common/graphics.c
    src/common/timer.h
    src/common/timer.c
)

target_include_directories(c8
    PRIVATE out/deps/SDL/include
    PRIVATE out/deps/SDL/include-config-relwithdebinfo
)

target_link_directories(c8
    PRIVATE out/deps/SDL/RelWithDebInfo
)

target_link_libraries(c8 PRIVATE SDL2)

# Assembler

add_executable(c8a
    src/assembler.c
    src/common/chip8.h
    src/common/chip8.c
    src/common/instructions.h
    src/common/instructions.c
    src/common/graphics.h
    src/common/graphics.c
)

target_include_directories(c8a
    PRIVATE out/deps/SDL/include
    PRIVATE out/deps/SDL/include-config-relwithdebinfo
)

target_link_directories(c8a
    PRIVATE out/deps/SDL/RelWithDebInfo
)

target_link_libraries(c8a PRIVATE SDL2)