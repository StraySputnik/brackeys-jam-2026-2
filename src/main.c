#include "game.h"

#include <raylib.h>

int main() {
    Game game = load_game("scene.txt", make_draw_config(56, 0, 20.0f));

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Game");
    SetExitKey(KEY_NULL);

    while (!WindowShouldClose()) {
        update_game(&game, GetFrameTime() * 5);

        BeginDrawing();
        ClearBackground(BLACK);
        draw_game(&game);
        handle_ui(&game);
        EndDrawing();
    }

    CloseWindow();
    unload_game(&game);
    return 0;
}
