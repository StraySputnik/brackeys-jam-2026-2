cmake_minimum_required(VERSION 3.0...4.0)
project(game C)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.0
)

FetchContent_MakeAvailable(raylib)

add_executable(game
        src/main.c
        include/grid.h
        src/grid.c
        include/entity.h
        src/entity.c
        include/scene.h
        src/scene.c
        include/common.h
)

target_include_directories(game PUBLIC include PRIVATE src)
target_link_libraries(game raylib)

target_compile_options(game PUBLIC
    -fsanitize=address
)

target_link_options(game PUBLIC
        -fsanitize=address
)