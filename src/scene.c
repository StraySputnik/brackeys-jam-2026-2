#include "scene.h"

#include <raymath.h>

#include <assert.h>
#include <stdio.h>

#define ENEMY_SPEED 50.0f
#define PROJECTILE_SPEED 1000.0f
#define TOWER_ATTACK_INTERVAL 4.0f

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

void unload_scene(Scene* scene) {
    delete_entity_pool(&scene->entity_pool);
    delete_grid(&scene->grid);
}

static Entity* get_closest_target(const Scene* scene, const Vector2 position) {
    float closest_distance = 10000000000.0f;
    Entity* closest_entity = NULL;

    for (size_t i = 0; i < scene->entity_pool.count; i++) {
        Entity* entity = &scene->entity_pool.entities[i];
        if (entity->type != ENEMY && entity->type != FRIEND) {
            continue;
        }

        if (entity->type == ENEMY && entity->data.enemy.marked) {
            continue;
        }

        if (entity->type == FRIEND && entity->data.friend.marked) {
            continue;
        }

        const float distance = Vector2Distance(position, entity->position);
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_entity = entity;
        }
    }

    return closest_entity;
}

static void update_entity(Entity* entity, Scene* scene, const float delta_time) {
    switch (entity->type) {
    case ENEMY:
        entity->position.x += entity->data.enemy.direction.x * ENEMY_SPEED * delta_time;
        entity->position.y += entity->data.enemy.direction.y * ENEMY_SPEED * delta_time;

        const int x = (int)entity->position.x / scene->draw_config.tile_size;
        const int y = (int)entity->position.y / scene->draw_config.tile_size;

        const Tile* tile = get_tile(&scene->grid, x, y);
        if (!tile) {
            fprintf(stderr, "Entity %lu is out of bounds at position (%f, %f)\n", entity->id, entity->position.x,
                    entity->position.y);
            despawn_entity(&scene->entity_pool, entity->id);
            return;
        }

        const Vector2 tile_pos = {
            .x = ((float)x + 0.5f) * (float)scene->draw_config.tile_size,
            .y = ((float)y + 0.5f) * (float)scene->draw_config.tile_size,
        };

        if (tile->type == CASTLE) {
            scene->game_state.castle_health -= 1;
            despawn_entity(&scene->entity_pool, entity->id);
            return;
        }

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

        break;
    case PROJECTILE:
        entity->position.x += entity->data.projectile.direction.x * PROJECTILE_SPEED * delta_time;
        entity->position.y += entity->data.projectile.direction.y * PROJECTILE_SPEED * delta_time;

        const Entity* target = get_closest_target(scene, entity->position);
        if (!target) {
            break;
        }

        if (Vector2Distance(entity->position, target->position) < scene->draw_config.entity_size) {
            despawn_entity(&scene->entity_pool, target->id);
            despawn_entity(&scene->entity_pool, entity->id);
            break;
        }

        break;
    default:
        break;
    }
}

static void update_tile(Tile* tile, Scene* scene, const float delta_time) {
    switch (tile->type) {
    case TOWER:
        {
            TowerData* data = &tile->data.tower;
            data->timer += delta_time;

            if (data->timer >= TOWER_ATTACK_INTERVAL) {
                const Vector2 position = {
                    .x = ((float)tile->x + 0.5f) * (float)scene->draw_config.tile_size,
                    .y = ((float)tile->y + 0.5f) * (float)scene->draw_config.tile_size,
                };

                const Entity* target = get_closest_target(scene, position);
                if (!target) {
                    break;
                }

                data->timer = 0.0f;

                Vector2 direction = Vector2Subtract(target->position, position);
                direction = Vector2Normalize(direction);

                const EntityId id = spawn_entity(&scene->entity_pool, position, PROJECTILE);
                get_entity(&scene->entity_pool, id)->data.projectile.direction = direction;
            }

            break;
        }
    case SPAWNER:
        {
            SpawnerData* data = &tile->data.spawner;
            data->timer += delta_time;

            if (data->timer >= data->spawn_interval) {
                data->timer = 0.0f;
                const Vector2 position = {
                    .x = ((float)tile->x + 0.5f) * (float)scene->draw_config.tile_size,
                    .y = ((float)tile->y + 0.5f) * (float)scene->draw_config.tile_size,
                };

                const EntityId id = spawn_entity(&scene->entity_pool, position, data->entity_type);
                if (data->entity_type == ENEMY) {
                    get_entity(&scene->entity_pool, id)->data.enemy.direction = data->direction;
                }

                if (data->entity_type == FRIEND) {
                    get_entity(&scene->entity_pool, id)->data.friend.direction = data->direction;
                }
            }

            break;
        }
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

static void update_tiles(Scene* scene, const float delta_time) {
    for (size_t i = 0; i < scene->grid.width * scene->grid.height; i++) {
        Tile* tile = &scene->grid.tiles[i];
        update_tile(tile, scene, delta_time);
    }
}

void update_scene(Scene* scene, const float delta_time) {
    update_entities(scene, delta_time);
    update_tiles(scene, delta_time);
}

void draw_scene(const Scene* scene) {
    draw_grid(&scene->grid, scene->draw_config.tile_size, scene->draw_config.tile_padding);
    draw_entities(&scene->entity_pool, scene->draw_config.entity_size);
}
