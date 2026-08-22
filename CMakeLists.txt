cmake_minimum_required(VERSION 3.0...4.0)
project(game)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.0
)

FetchContent_MakeAvailable(raylib)

add_executable(game
        src/main.c
)

target_link_libraries(game raylib)