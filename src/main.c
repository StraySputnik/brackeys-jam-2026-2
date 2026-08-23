#include "scene.h"

#include <raylib.h>

int main() {
    Scene scene = load_scene("scene.txt");
    InitWindow(1280, 720, "Game");

    spawn_entity(&scene.entity_pool, (Vector2){.x = 96, .y = 96}, ENEMY);
    get_tile(&scene.grid, 4, 4)->type = TOWER;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_scene(&scene, 64, 4, 24);

        EndDrawing();
    }

    CloseWindow();
    delete_scene(&scene);
    return 0;
}
