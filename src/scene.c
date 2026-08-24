#include "scene.h"

#include <raymath.h>

#include <assert.h>
#include <stdio.h>

DrawConfig make_draw_config(const int tile_size, const int tile_padding, const float entity_size) {
    DrawConfig config;
    config.tile_size = tile_size;
    config.tile_padding = tile_padding;
    config.entity_size = entity_size;
    return config;
}

static void get_file_dimensions(FILE* file, int* width, int* height) {
    int w = 0;
    int h = 0;
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

    *width = max_w;
    *height = h + 1;
}

Scene load_scene(const char* filename, const DrawConfig draw_config) {
    Scene scene;
    scene.entity_pool = make_entity_pool();
    scene.draw_config = draw_config;
    scene.game_state.castle_health = 100;

    FILE* file = fopen(filename, "r");
    assert(file && "Failed to open file.");

    int width, height;
    get_file_dimensions(file, &width, &height);

    scene.grid = make_grid(width, height);

    int x = 0;
    int y = 0;

    rewind(file);

    int c = fgetc(file);
    while (c != EOF) {
        switch (c) {
        case 'G':
            get_tile(&scene.grid, x, y)->type = GRASS;
            break;
        case 'P':
            get_tile(&scene.grid, x, y)->type = PATH;
            break;
        case 'U':
            get_tile(&scene.grid, x, y)->type = UP;
            break;
        case 'D':
            get_tile(&scene.grid, x, y)->type = DOWN;
            break;
        case 'L':
            get_tile(&scene.grid, x, y)->type = LEFT;
            break;
        case 'R':
            get_tile(&scene.grid, x, y)->type = RIGHT;
            break;
        case 'H':
            get_tile(&scene.grid, x, y)->type = CASTLE;
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

#define ENEMY_SPEED 50.0f

static void update_entity(Entity* entity, Scene* scene, const float delta_time) {
    switch (entity->type) {
    case ENEMY:
        entity->position.x += entity->data.enemy.direction.x * ENEMY_SPEED * delta_time;
        entity->position.y += entity->data.enemy.direction.y * ENEMY_SPEED * delta_time;

        const int x = (int)entity->position.x / scene->draw_config.tile_size;
        const int y = (int)entity->position.y / scene->draw_config.tile_size;

        const Tile* tile = get_tile(&scene->grid, x, y);
        assert(tile && "Entity is out of bounds.");

        const Vector2 tile_pos = {
            .x = ((float)x + 0.5f) * (float)scene->draw_config.tile_size,
            .y = ((float)y + 0.5f) * (float)scene->draw_config.tile_size,
        };

        if (Vector2Distance(entity->position, tile_pos) > 0.02f * (float)scene->draw_config.tile_size) {
            return;
        }

        if (tile->type == UP) {
            entity->data.enemy.direction.y = -1;
            entity->data.enemy.direction.x = 0;
        }

        if (tile->type == DOWN) {
            entity->data.enemy.direction.y = 1;
            entity->data.enemy.direction.x = 0;
        }

        if (tile->type == RIGHT) {
            entity->data.enemy.direction.y = 0;
            entity->data.enemy.direction.x = 1;
        }

        if (tile->type == LEFT) {
            entity->data.enemy.direction.y = 0;
            entity->data.enemy.direction.x = -1;
        }

        if (tile->type == CASTLE) {
            scene->game_state.castle_health -= 1;
            despawn_entity(&scene->entity_pool, entity->id);
            return;
        }

        break;
    default:
        break;
    }
}

static void update_entities(Scene* scene, const float delta_time) {
    for (size_t i = 0; i < scene->entity_pool.count; i++) {
        Entity* entity = &scene->entity_pool.entities[i];
        update_entity(entity, scene, delta_time);
    }
}

void update_scene(Scene* scene, const float delta_time) {
    update_entities(scene, delta_time);
}

void draw_scene(const Scene* scene) {
    draw_grid(&scene->grid, scene->draw_config.tile_size, scene->draw_config.tile_padding);
    draw_entities(&scene->entity_pool, scene->draw_config.entity_size);
}
