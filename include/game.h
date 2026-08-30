#ifndef GAME_GAME_H
#define GAME_GAME_H

#include "audio.h"
#include "scene.h"

typedef struct {
    int   tile_size;
    int   tile_padding;
    float entity_size;
} DrawConfig;

DrawConfig make_draw_config(int tile_size, int tile_padding, float entity_size);

typedef struct {
    TileType selected_building_type;
    Entity * selected_entity;
} UIState;

int draw_button(const char *label, int x, int y, int width, int height);

typedef struct {
    const char *filename;
    Scene       scene;
    DrawConfig  draw_config;
    UIState     ui_state;
    AudioStore  audio_store;
    Camera2D    camera;
    int         castle_health;
    int         coins;
    int         current_wave;
    float       pause_timer;
    int         scene_idx;
} Game;

Game load_game(const char *filename, DrawConfig draw_config);
void unload_game(Game *game);

int  update_game(Game *game, float delta_time);
void draw_game(const Game *game);
void handle_ui(Game *game);

#endif //GAME_GAME_H
