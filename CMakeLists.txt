cmake_minimum_required(VERSION 3.7)

project(chip8)

add_subdirectory(deps/SDL)

add_executable(chip8 src/main.c)

target_include_directories(chip8
    PRIVATE out/deps/SDL/include
    PRIVATE out/deps/SDL/include-config-relwithdebinfo
)

target_link_directories(chip8
    PRIVATE out/deps/SDL/RelWithDebInfo
)

target_link_libraries(chip8 PRIVATE SDL2)