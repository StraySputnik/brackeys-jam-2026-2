#include "scene.h"

#include <raymath.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void get_file_dimensions(FILE *file, int *width, int *height) {
    int w     = 0;
    int h     = 0;
    int max_w = 0;

    int c = fgetc(file);
    while (c != EOF) {
        if (c == '\n') {
            h++;
            w = 0;
        } else {
            w++;
            if (w > max_w) {
                max_w = w;
            }
        }

        c = fgetc(file);
    }

    *width  = max_w;
    *height = h + 1;

    rewind(file);
}

static void load_wave(FILE *file, Scene *scene, const int wave_idx) {
    SpawnQueue *queues = malloc(SPAWNER_COUNT * sizeof(SpawnQueue));
    for (int i = 0; i < SPAWNER_COUNT; i++) {
        queues[i] = make_spawn_queue();
    }

    int c   = fgetc(file);
    int idx = 0;
    while (c != EOF && idx < SPAWNER_COUNT) {
        if (c == '-') {
            idx++;
        }

        if (c == 'v') {
            spawn_queue_push(&queues[idx], MONSTER_VAMPIRE);
        }

        if (c == 'w') {
            spawn_queue_push(&queues[idx], MONSTER_WEREWOLF);
        }

        if (c == 'g') {
            spawn_queue_push(&queues[idx], MONSTER_GHOST);
        }

        if (c == '0') {
            spawn_queue_push(&queues[idx], MONSTER_NONE);
        }

        c = fgetc(file);
    }

    int spawner_idx = 0;
    for (int i = 0; i < scene->grid.width; i++) {
        for (int j = 0; j < scene->grid.height; j++) {
            Tile *tile = get_tile(&scene->grid, i, j);
            if (tile->type != TILE_SPAWNER) {
                continue;
            }

            if (!tile->data.s.waves) {
                tile->data.s.waves      = malloc(wave_idx * sizeof(SpawnQueue));
                tile->data.s.wave_count = wave_idx;
                for (int w = 0; w < wave_idx - 1; w++) {
                    tile->data.s.waves[w] = make_spawn_queue();
                }
            }

            if (wave_idx >= tile->data.s.wave_count) {
                tile->data.s.waves      = realloc(tile->data.s.waves, wave_idx * sizeof(SpawnQueue));
                tile->data.s.wave_count = wave_idx;
            }

            tile->data.s.waves[wave_idx - 1] = queues[spawner_idx];
            tile->data.s.friendly_count      = (int)queues[spawner_idx].size / 2;
            spawner_idx++;
        }
    }
}

static void load_waves(Scene *scene) {
    int i = 1;
    while (true) {
        char *filename = malloc(32);
        sprintf(filename, "res/wave%c.txt", '0' + i);
        FILE *file = fopen(filename, "r");
        if (!file || i > WAVE_COUNT) {
            break;
        }

        load_wave(file, scene, i);
        i++;
    }
}

Scene load_scene(const char *filename) {
    Scene scene;
    scene.entity_pool = make_entity_pool();

    FILE *file = fopen(filename, "r");
    assert(file && "Failed to open file.");

    int width, height;
    get_file_dimensions(file, &width, &height);

    scene.grid = make_grid(width, height);

    int x = 0;
    int y = 0;

    int c = fgetc(file);
    while (c != EOF) {
        switch (c) {
        case '.':
            get_tile(&scene.grid, x, y)->type = TILE_GRASS;
            break;
        case 'P':
            get_tile(&scene.grid, x, y)->type = TILE_PATH;
            break;
        case 'U':
            get_tile(&scene.grid, x, y)->type = TILE_UP;
            break;
        case 'D':
            get_tile(&scene.grid, x, y)->type = TILE_DOWN;
            break;
        case 'L':
            get_tile(&scene.grid, x, y)->type = TILE_LEFT;
            break;
        case 'R':
            get_tile(&scene.grid, x, y)->type = TILE_RIGHT;
            break;
        case 'H':
            get_tile(&scene.grid, x, y)->type = TILE_CASTLE;
            break;
        case 'T':
            get_tile(&scene.grid, x, y)->type = TILE_TOWER;
            break;
        case 'C':
            get_tile(&scene.grid, x, y)->type = TILE_COLLECTOR;
            break;
        case '^':
            get_tile(&scene.grid, x, y)->type = TILE_SPAWNER;
            get_tile(&scene.grid, x, y)->data.s.waves          = NULL;
            get_tile(&scene.grid, x, y)->data.s.wave_count     = 0;
            get_tile(&scene.grid, x, y)->data.s.timer          = 0;
            get_tile(&scene.grid, x, y)->data.s.friendly_count = 0;
            get_tile(&scene.grid, x, y)->data.s.direction      = (Vector2){.x = 0, .y = -1};
            get_tile(&scene.grid, x, y)->data.s.finished       = false;
            break;
        case '>':
            get_tile(&scene.grid, x, y)->type = TILE_SPAWNER;
            get_tile(&scene.grid, x, y)->data.s.waves          = NULL;
            get_tile(&scene.grid, x, y)->data.s.wave_count     = 0;
            get_tile(&scene.grid, x, y)->data.s.timer          = 0;
            get_tile(&scene.grid, x, y)->data.s.friendly_count = 0;
            get_tile(&scene.grid, x, y)->data.s.direction      = (Vector2){.x = 1, .y = 0};
            get_tile(&scene.grid, x, y)->data.s.finished       = false;
            break;
        case '<':
            get_tile(&scene.grid, x, y)->type = TILE_SPAWNER;
            get_tile(&scene.grid, x, y)->data.s.waves          = NULL;
            get_tile(&scene.grid, x, y)->data.s.wave_count     = 0;
            get_tile(&scene.grid, x, y)->data.s.timer          = 0;
            get_tile(&scene.grid, x, y)->data.s.friendly_count = 0;
            get_tile(&scene.grid, x, y)->data.s.direction      = (Vector2){.x = -1, .y = 0};
            get_tile(&scene.grid, x, y)->data.s.finished       = false;
            break;
        case '!':
            get_tile(&scene.grid, x, y)->type = TILE_SPAWNER;
            get_tile(&scene.grid, x, y)->data.s.waves          = NULL;
            get_tile(&scene.grid, x, y)->data.s.wave_count     = 0;
            get_tile(&scene.grid, x, y)->data.s.timer          = 0;
            get_tile(&scene.grid, x, y)->data.s.friendly_count = 0;
            get_tile(&scene.grid, x, y)->data.s.direction      = (Vector2){.x = 0, .y = 1};
            get_tile(&scene.grid, x, y)->data.s.finished       = false;
            break;
        default:
            break;
        }

        if (c == '\n') {
            y++;
            x = 0;
            c = fgetc(file);
            continue;
        }

        x++;
        c = fgetc(file);
    }

    fclose(file);

    load_waves(&scene);
    return scene;
}

void unload_scene(Scene *scene) {
    delete_entity_pool(&scene->entity_pool);
    delete_grid(&scene->grid);
}

void draw_scene(const SpriteStore *sprite_store, const Scene *scene, const int tile_size, const int tile_padding,
                const float        entity_size) {
    draw_grid(sprite_store, &scene->grid, tile_size, tile_padding);
    draw_entities(sprite_store, &scene->entity_pool, entity_size);
}
