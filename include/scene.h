#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "grid.h"
#include "entity.h"

typedef struct {
    int tile_size;
    int tile_padding;
    float entity_size;
} DrawConfig;

typedef struct {
    int castle_health;
} GameState;

DrawConfig make_draw_config(int tile_size, int tile_padding, float entity_size);

typedef struct {
    Grid grid;
    EntityPool entity_pool;
    DrawConfig draw_config;
    GameState game_state;
} Scene;

Scene load_scene(const char* filename, DrawConfig draw_config);
void unload_scene(Scene* scene);

void update_scene(Scene* scene, float delta_time);
void draw_scene(const Scene* scene);

#endif //GAME_SCENE_H
