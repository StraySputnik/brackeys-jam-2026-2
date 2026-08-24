#include "scene.h"

#include <raylib.h>

int main() {
    Scene scene = load_scene("scene.txt", make_draw_config(64, 0, 24.0f));

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Game");

    Tile *spawner = get_tile(&scene.grid, 0, 1);

    spawner->type = SPAWNER;
    spawner->data.spawner.direction = (Vector2){.x = 1, .y = 0};
    spawner->data.spawner.entity_type = ENEMY;
    spawner->data.spawner.timer = 0.0f;
    spawner->data.spawner.spawn_interval = 2.0f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        update_scene(&scene, GetFrameTime());
        draw_scene(&scene);

        EndDrawing();
    }

    CloseWindow();
    unload_scene(&scene);
    return 0;
}
