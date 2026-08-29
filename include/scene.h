#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "grid.h"
#include "entity.h"

#define NUM_SPAWNERS 6
#define NUM_WAVES    2

typedef struct {
    Grid       grid;
    EntityPool entity_pool;
} Scene;

Scene load_scene(const char *filename);
void  unload_scene(Scene *scene);

void draw_scene(const Scene *scene, int tile_size, int tile_padding, float entity_size);

#endif //GAME_SCENE_H
