#include "scene.h"

#include <raylib.h>

int main() {
    Scene scene = load_scene("scene.txt", make_draw_config(64, 4, 24.0f));
    InitWindow(1280, 720, "Game");

    const int id = spawn_entity(&scene.entity_pool, (Vector2){.x = 96, .y = 96}, ENEMY);
    get_entity(&scene.entity_pool, id)->data.enemy.direction = (Vector2){.x = 1, .y = 0};

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        update_scene(&scene, GetFrameTime());
        draw_scene(&scene);

        EndDrawing();
    }

    CloseWindow();
    delete_scene(&scene);
    return 0;
}
