#include "scene.h"

#include <assert.h>
#include <stdio.h>

static void get_file_dimensions(FILE* file, int* width, int* height) {
    *width = 0;
    *height = 0;

    int w = 0;
    int h = 0;
    int max_w = 0;

    char c = fgetc(file);
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

    *width = max_w;
    *height = h + 1;
}

Scene load_scene(const char* filename) {
    Scene scene;
    scene.entity_pool = make_entity_pool();

    FILE* file = fopen(filename, "r");
    assert(file && "Failed to open file.");

    int width, height;
    get_file_dimensions(file, &width, &height);

    scene.grid = make_grid(width, height);

    int x = 0;
    int y = 0;

    rewind(file);

    char c = fgetc(file);
    while (c != EOF) {
        switch (c) {
        case 'G':
            get_tile(&scene.grid, x, y)->type = GRASS;
            break;
        case 'P':
            get_tile(&scene.grid, x, y)->type = PATH;
            break;
        case 'L':
            get_tile(&scene.grid, x, y)->type = LEFT_TURN;
            break;
        case 'R':
            get_tile(&scene.grid, x, y)->type = RIGHT_TURN;
            break;
        case 'T':
            get_tile(&scene.grid, x, y)->type = TOWER;
            break;
        case 'C':
            get_tile(&scene.grid, x, y)->type = COLLECTOR;
            break;
        case 'S':
            get_tile(&scene.grid, x, y)->type = SPAWNER;
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
    return scene;
}

void delete_scene(Scene* scene) {
    delete_entity_pool(&scene->entity_pool);
    delete_grid(&scene->grid);
}

void draw_scene(const Scene* scene, const int tile_size, const int tile_padding, const float entity_size) {
    draw_grid(&scene->grid, tile_size, tile_padding);
    draw_entities(&scene->entity_pool, entity_size);
}
