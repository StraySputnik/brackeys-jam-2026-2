#include "game.h"

#include <raylib.h>

int main() {
    Game game = load_game("scene.txt", make_draw_config(64, 0, 24.0f));

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Game");
    SetExitKey(KEY_NULL);

    Tile* spawner = get_tile(&game.scene.grid, 0, 1);

    spawner->type = SPAWNER;
    spawner->data.spawner.direction = (Vector2){.x = 1, .y = 0};
    spawner->data.spawner.entity_type = ENEMY;
    spawner->data.spawner.timer = 0.0f;
    spawner->data.spawner.spawn_interval = 2.0f;\

    while (!WindowShouldClose()) {
        update_game(&game, GetFrameTime());

        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_game(&game);
        handle_ui(&game);
        EndDrawing();
    }

    CloseWindow();
    unload_game(&game);
    return 0;
}
