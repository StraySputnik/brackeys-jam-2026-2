#include "game.h"

#include <raylib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static void UpdateDrawFrame();

static Game  game;
static Sound music;

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Knight Guard");
    InitAudioDevice();
    SetExitKey(KEY_NULL);

    game  = load_game("res/scene.txt", make_draw_config(56, 0, 20.0f));
    music = LoadSound("res/music/LevelMusic.ogg");
    PlaySound(music);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    UnloadSound(music);
    CloseAudioDevice();
    CloseWindow();
    unload_game(&game);
    return 0;
}

void UpdateDrawFrame() {
    if (!IsSoundPlaying(music)) {
        PlaySound(music);
    }

    if (update_game(&game, GetFrameTime()) == -1) {
        return;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    draw_game(&game);
    handle_ui(&game);
    EndDrawing();
}
