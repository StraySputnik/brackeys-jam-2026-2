#include "game.h"

#include "raymath.h"

#include <stdio.h>

#define INITIAL_CASTLE_HEALTH 10
#define ENEMY_SPEED           1.0f
#define PROJECTILE_SPEED      16.0f
#define TOWER_ATTACK_INTERVAL 4.0f
#define TOWER_RANGE           3.0f

static int build_price(const TileType type) {
    switch (type) {
    case TOWER:
        return 5;
    default:
        return 1023;
    }
}

static Entity *get_closest_target(const Game *game, const Vector2 position) {
    float   closest_distance = 10000000000.0f;
    Entity *closest_entity   = NULL;

    for (size_t i = 0; i < game->scene.entity_pool.count; i++) {
        Entity *entity = &game->scene.entity_pool.entities[i];
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
            closest_entity   = entity;
        }
    }

    return closest_entity;
}

static void update_monster(Entity *entity, Game *game, const float delta_time) {
    const EntityType type = entity->type;
    if (type != ENEMY && type != FRIEND) {
        return;
    }

    entity->position.x +=
        entity->data.enemy.direction.x * ENEMY_SPEED * (float)game->draw_config.tile_size * delta_time;
    entity->position.y +=
        entity->data.enemy.direction.y * ENEMY_SPEED * (float)game->draw_config.tile_size * delta_time;

    const int x = (int)entity->position.x / game->draw_config.tile_size;
    const int y = (int)entity->position.y / game->draw_config.tile_size;

    const Tile *tile = get_tile(&game->scene.grid, x, y);
    if (!tile) {
        fprintf(
            stderr,
            "Entity %lu is out of bounds at position (%f, %f)\n", entity->id, entity->position.x,
            entity->position.y
        );

        despawn_entity(&game->scene.entity_pool, entity->id);
        return;
    }

    const Vector2 tile_pos = {
        .x = ((float)x + 0.5f) * (float)game->draw_config.tile_size,
        .y = ((float)y + 0.5f) * (float)game->draw_config.tile_size,
    };

    if (tile->type == CASTLE) {
        if (type == ENEMY) {
            game->castle_health -= 1;
        }

        if (type == FRIEND) {
            game->resources += 1;
        }

        despawn_entity(&game->scene.entity_pool, entity->id);
        return;
    }

    if (Vector2Distance(entity->position, tile_pos) > 0.02f * (float)game->draw_config.tile_size) {
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
}

static void update_entity(Entity *entity, Game *game, const float delta_time) {
    switch (entity->type) {
    case ENEMY:
        update_monster(entity, game, delta_time);
        break;
    case FRIEND:
        update_monster(entity, game, delta_time);
        break;
    case PROJECTILE:
        entity->position.x += entity->data.projectile.direction.x * PROJECTILE_SPEED * (float)game->draw_config.
            tile_size * delta_time;
        entity->position.y += entity->data.projectile.direction.y * PROJECTILE_SPEED * (float)game->draw_config.
            tile_size * delta_time;

        if (entity->position.x < -1000 || entity->position.x > 5000 || entity->position.y < -1000 || entity->position.y
            > 5000) {
            despawn_entity(&game->scene.entity_pool, entity->id);
            break;
        }

        const Entity *target = get_closest_target(game, entity->position);
        if (!target) {
            break;
        }

        if (Vector2Distance(entity->position, target->position) < game->draw_config.entity_size) {
            despawn_entity(&game->scene.entity_pool, target->id);
            despawn_entity(&game->scene.entity_pool, entity->id);
            break;
        }

        break;
    default:
        break;
    }
}

static void update_tile(Tile *tile, Game *game, const float delta_time) {
    switch (tile->type) {
    case TOWER:
        {
            TowerData *data = &tile->data.tower;
            data->timer     += delta_time;

            if (data->timer >= TOWER_ATTACK_INTERVAL) {
                const Vector2 position = {
                    .x = ((float)tile->x + 0.5f) * (float)game->draw_config.tile_size,
                    .y = ((float)tile->y + 0.5f) * (float)game->draw_config.tile_size,
                };

                const Entity *target = get_closest_target(game, position);
                if (!target) {
                    break;
                }

                if (Vector2Distance(position, target->position) > TOWER_RANGE * (float)game->draw_config.tile_size) {
                    break;
                }

                data->timer = 0.0f;

                Vector2 direction = Vector2Subtract(target->position, position);
                direction         = Vector2Normalize(direction);

                const EntityId id = spawn_entity(&game->scene.entity_pool, position, PROJECTILE);
                get_entity(&game->scene.entity_pool, id)->data.projectile.direction = direction;
            }

            break;
        }
    case SPAWNER:
        {
            SpawnerData *data = &tile->data.spawner;
            data->timer       += delta_time;

            if (data->timer >= data->spawn_interval) {
                data->timer            = 0.0f;
                const Vector2 position = {
                    .x = ((float)tile->x + 0.5f) * (float)game->draw_config.tile_size,
                    .y = ((float)tile->y + 0.5f) * (float)game->draw_config.tile_size,
                };

                const EntityId id = spawn_entity(&game->scene.entity_pool, position, data->entity_type);
                if (data->entity_type == ENEMY) {
                    get_entity(&game->scene.entity_pool, id)->data.enemy.direction = data->direction;
                    get_entity(&game->scene.entity_pool, id)->data.enemy.marked    = false;
                }

                if (data->entity_type == FRIEND) {
                    get_entity(&game->scene.entity_pool, id)->data.friend.direction = data->direction;
                    get_entity(&game->scene.entity_pool, id)->data.friend.marked    = false;
                }
            }

            break;
        }
    default:
        break;
    }
}

static void update_entities(Game *game, const float delta_time) {
    for (size_t i = 0; i < game->scene.entity_pool.count; i++) {
        Entity *entity = &game->scene.entity_pool.entities[i];
        update_entity(entity, game, delta_time);
    }
}

static void update_tiles(Game *game, const float delta_time) {
    for (size_t i = 0; i < game->scene.grid.width * game->scene.grid.height; i++) {
        Tile *tile = &game->scene.grid.tiles[i];
        update_tile(tile, game, delta_time);
    }
}

DrawConfig make_draw_config(const int tile_size, const int tile_padding, const float entity_size) {
    DrawConfig config;
    config.tile_size    = tile_size;
    config.tile_padding = tile_padding;
    config.entity_size  = entity_size;
    return config;
}

int draw_button(const char *label, const int x, const int y, const int width, const int height) {
    const Rectangle button_rect = {.x = x, .y = y, .width = width, .height = height};
    const Rectangle border_rect = {.x = x - 2, .y = y - 2, .width = width + 4, .height = height + 4};

    Color       button_color = LIGHTGRAY;
    const Color text_color   = BLACK;

    int out = 0;
    if (CheckCollisionPointRec(GetMousePosition(), button_rect)) {
        button_color = GRAY;
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            button_color = RAYWHITE;
            out          = 1;
        }
        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            out = 2;
        }
    }

    DrawRectangleRec(border_rect, DARKGRAY);
    DrawRectangleRec(button_rect, button_color);
    DrawText(label, x + 10, y + height / 4, 20, text_color);

    return out;
}

Game load_game(const char *filename, const DrawConfig draw_config) {
    Game game;
    game.scene                           = load_scene(filename);
    game.draw_config                     = draw_config;
    game.ui_state.selected_building_type = GRASS;
    game.castle_health                   = INITIAL_CASTLE_HEALTH;
    game.resources                       = 10;

    const int   grid_width         = game.scene.grid.width * game.draw_config.tile_size / 2;
    const int   grid_height        = game.scene.grid.height * game.draw_config.tile_size / 2;
    const float half_screen_width  = 1280.0f / 2.0f;
    const float half_screen_height = 720.0f / 2.0f;

    game.camera = (Camera2D){
        .offset = (Vector2){
            .x = 0.0f, .y = 0.0f
        },
        .rotation = 0.0f,
        .target   = (Vector2){
            .x = (float)grid_width - (float)half_screen_width,
            .y = (float)grid_height - (float)half_screen_height
        },
        .zoom = 1.0f,
    };

    return game;
}

void unload_game(Game *game) {
    unload_scene(&game->scene);
}

void update_game(Game *game, const float delta_time) {
    update_tiles(game, delta_time);
    update_entities(game, delta_time);
}

void draw_game(const Game *game) {
    BeginMode2D(game->camera);
    draw_scene(
        &game->scene, game->draw_config.tile_size,
        game->draw_config.tile_padding,
        game->draw_config.entity_size
    );
    EndMode2D();
}

void handle_ui(Game *game) {
    DrawText(TextFormat("HP: %d", game->castle_health), 10, 10, 20, RED);
    DrawText(TextFormat("Resources: %d", game->resources), 80, 10, 20, YELLOW);

    if (draw_button("Build Tower", 10, 40, 150, 40)) {
        game->ui_state.selected_building_type = TOWER;
    }

    const TileType building = game->ui_state.selected_building_type;
    if (building == GRASS) {
        return;
    }

    if (game->resources < build_price(building)) {
        game->ui_state.selected_building_type = GRASS;
        return;
    }

    BeginMode2D(game->camera);

    const Vector2 mouse_pos = GetMousePosition();
    const int     x         = (int)(mouse_pos.x + game->camera.target.x) / game->draw_config.tile_size;
    const int     y         = (int)(mouse_pos.y + game->camera.target.y) / game->draw_config.tile_size;

    Color placeholder_color = GREEN;
    Tile *tile              = get_tile(&game->scene.grid, x, y);

    if (!tile || tile->type != GRASS) {
        placeholder_color = RED;
    }

    const int tile_size    = game->draw_config.tile_size;
    const int tile_padding = game->draw_config.tile_padding;

    if (building == TOWER) {
        DrawCircle(
            x * tile_size + tile_size / 2,
            y * tile_size + tile_size / 2,
            TOWER_RANGE * (float)tile_size,
            (Color){
                .r = placeholder_color.r, .g = placeholder_color.g, .b = placeholder_color.b, .a = 100
            }
        );
    }

    DrawRectangle(
        x * tile_size + tile_padding,
        y * tile_size + tile_padding,
        tile_size - tile_padding * 2,
        tile_size - tile_padding * 2,
        placeholder_color
    );

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        if (tile && tile->type == GRASS) {
            game->resources -= build_price(building);
            tile->type      = building;
            if (building == TOWER) {
                tile->data.tower.timer = 0.0f;
            }
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        game->ui_state.selected_building_type = GRASS;
    }

    EndMode2D();
}
