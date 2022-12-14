# For IF generator expressions
cmake_minimum_required(VERSION 3.8)

# Configuration in all lower case (for including SDL config)
set(config_lower string(TOLOWER $<CONFIG>))

project(chip8)

add_subdirectory(deps/SDL)

# Emulator

add_executable(c8
    src/emulator.c
    src/common/chip8.h
    src/common/chip8.c
    src/common/instructions.h
    src/common/instructions.c
    src/common/platform.h
    src/common/platform.c
    src/common/timer.h
    src/common/timer.c
)

target_include_directories(c8
    PRIVATE out/deps/SDL/include
    PRIVATE out/deps/SDL/include-config/$(config_lower)
)

target_link_directories(c8
    PRIVATE out/deps/SDL/$<CONFIG>
)

target_link_libraries(c8 PRIVATE SDL2)

# Assembler

add_executable(c8a
    src/assembler.c
    src/common/chip8.h
    src/common/chip8.c
    src/common/instructions.h
    src/common/instructions.c
    src/common/platform.h
    src/common/platform.c
)

target_include_directories(c8a
    PRIVATE out/deps/SDL/include
    PRIVATE out/deps/SDL/include-config/$(config_lower)
)

target_link_directories(c8a
    PRIVATE out/deps/SDL/$<CONFIG>
)

target_link_libraries(c8a PRIVATE SDL2)

# Copy SDL into release file

add_custom_command(TARGET c8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE_DIR:SDL2>/SDL2.dll
    $<TARGET_FILE_DIR:c8>
)