#include "game.h"

#include <raylib.h>

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Game");
    InitAudioDevice();
    SetExitKey(KEY_NULL);

    Game game = load_game("scene.txt", make_draw_config(56, 0, 20.0f));

    const Sound music = LoadSound("music/LevelMusic.ogg");
    PlaySound(music);

    while (!WindowShouldClose()) {
        if (!IsSoundPlaying(music)) {
            PlaySound(music);
        }

        update_game(&game, GetFrameTime());

        BeginDrawing();
        ClearBackground(BLACK);
        draw_game(&game);
        handle_ui(&game);
        EndDrawing();
    }

    UnloadSound(music);

    CloseAudioDevice();
    CloseWindow();
    unload_game(&game);
    return 0;
}
