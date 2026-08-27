#include "game.h"
#include "ui.h"

#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CASTLE_HEALTH 10
#define ENEMY_SPEED           1
#define PROJECTILE_SPEED      16
#define TOWER_ATTACK_INTERVAL 4
#define TOWER_RANGE           3

static const char *random_vampire_name() {
    const char *names[] = {
        "Dracula",
        "Vlad",
        "Draven",
        "Elizabeth",
        "Crimson",
        "Lazarus",
        "Scarlett",
        "Selene",
        "Igor",
        "Chocula"
    };

    return names[GetRandomValue(0, sizeof(names) / sizeof(names[0]) - 1)];
}

static const char *random_werewolf_name() {
    const char *names[] = {
        "Fenrir",
        "Remus",
        "Cynric",
        "Wren",
        "Lyra",
        "Hunter",
        "Fleabag",
        "Wolferine",
    };

    return names[GetRandomValue(0, sizeof(names) / sizeof(names[0]) - 1)];
}

static const char *random_ghost_name() {
    const char *names[] = {
        "Liam",
        "Celeste",
        "Mary",
        "Blair",
        "Specter",
        "Agnes",
    };

    return names[GetRandomValue(0, sizeof(names) / sizeof(names[0]) - 1)];
}

static Entity *get_closest_target(const Game *game, const Vector2 position) {
    float   closest_distance = 10000000000.0f;
    Entity *closest_entity   = NULL;

    for (size_t i = 0; i < game->scene.entity_pool.count; i++) {
        Entity *entity = &game->scene.entity_pool.entities[i];
        if (entity->type != ENTITY_MONSTER || !entity->data.m.marked) {
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

static void randomize_monster_data(MonsterData *monster, const MonsterType type, const bool friendly) {
    monster->monster_type = type;

    if (friendly) {
        switch (type) {
        case MONSTER_WEREWOLF:
            monster->name = random_werewolf_name();
            monster->title        = TITLE_NONE;
            monster->age          = GetRandomValue(18, 60);
            monster->weakness     = WEAKNESS_SILVER;
            monster->favored_food = FOOD_MEAT;
            break;
        case MONSTER_VAMPIRE:
            monster->name = random_vampire_name();
            monster->title        = TITLE_NONE + GetRandomValue(0, 1);
            monster->age          = GetRandomValue(18, 800);
            monster->weakness     = WEAKNESS_GARLIC + GetRandomValue(0, 1);
            monster->favored_food = FOOD_BLOOD;
            break;
        case MONSTER_GHOST:
            monster->name = random_ghost_name();
            monster->title        = TITLE_NONE;
            monster->age          = GetRandomValue(18, 2000);
            monster->weakness     = WEAKNESS_HOLY_WATER + GetRandomValue(0, 1);
            monster->favored_food = FOOD_NONE;
            break;
        default:
            break;
        }
    } else {
        do {
            monster->title        = GetRandomValue(0, TITLE_COUNT - 1);
            monster->weakness     = GetRandomValue(0, WEAKNESS_TYPE_COUNT - 1);
            monster->favored_food = GetRandomValue(0, FOOD_TYPE_COUNT - 1);

            const int age_group = GetRandomValue(0, 2);
            switch (age_group) {
            case 0:
                monster->age = GetRandomValue(18, 60);
                break;
            case 1:
                monster->age = GetRandomValue(18, 800);
                break;
            default:
                monster->age = GetRandomValue(18, 2000);
                break;
            }

            const int name_group = GetRandomValue(0, 2);
            switch (name_group) {
            case 0:
                monster->name = random_vampire_name();
                break;
            case 1:
                monster->name = random_werewolf_name();
                break;
            default:
                monster->name = random_ghost_name();
                break;
            }
        } while (is_friendly(*monster));
    }
}

static void update_entity(Entity *entity, Game *game, const float delta_time) {
    switch (entity->type) {
    case ENTITY_MONSTER:
        entity->position.x +=
            entity->data.m.direction.x * ENEMY_SPEED * (float)game->draw_config.tile_size * delta_time;
        entity->position.y +=
            entity->data.m.direction.y * ENEMY_SPEED * (float)game->draw_config.tile_size * delta_time;

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

        if (tile->type == TILE_CASTLE) {
            if (is_friendly(entity->data.m)) {
                game->resources += 1;
            } else {
                game->castle_health -= 1;
            }

            despawn_entity(&game->scene.entity_pool, entity->id);
            return;
        }

        if (Vector2Distance(entity->position, tile_pos) > 0.02f * (float)game->draw_config.tile_size) {
            return;
        }

        if (tile->type == TILE_UP) {
            entity->data.m.direction.y = -1;
            entity->data.m.direction.x = 0;
        }

        if (tile->type == TILE_DOWN) {
            entity->data.m.direction.y = 1;
            entity->data.m.direction.x = 0;
        }

        if (tile->type == TILE_RIGHT) {
            entity->data.m.direction.y = 0;
            entity->data.m.direction.x = 1;
        }

        if (tile->type == TILE_LEFT) {
            entity->data.m.direction.y = 0;
            entity->data.m.direction.x = -1;
        }
        break;
    case ENTITY_PROJECTILE:
        entity->position.x +=
            entity->data.p.direction.x * PROJECTILE_SPEED * (float)game->draw_config.tile_size * delta_time;
        entity->position.y +=
            entity->data.p.direction.y * PROJECTILE_SPEED * (float)game->draw_config.tile_size * delta_time;

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
    case TILE_TOWER:
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

                const EntityId id = spawn_entity(&game->scene.entity_pool, position, ENTITY_PROJECTILE);
                get_entity(&game->scene.entity_pool, id)->data.p.direction = direction;
            }

            break;
        }
    case TILE_SPAWNER:
        {
            SpawnerData *data = &tile->data.spawner;
            data->timer       += delta_time;

            if (data->timer >= data->spawn_interval) {
                data->timer            = 0.0f;
                const Vector2 position = {
                    .x = ((float)tile->x + 0.5f) * (float)game->draw_config.tile_size,
                    .y = ((float)tile->y + 0.5f) * (float)game->draw_config.tile_size,
                };

                const EntityId id = spawn_entity(&game->scene.entity_pool, position, ENTITY_MONSTER);
                MonsterData *  m  = &get_entity(&game->scene.entity_pool, id)->data.m;

                m->direction    = data->direction;
                m->monster_type = data->monster_type;
                m->marked       = false;
                randomize_monster_data(m, m->monster_type, true);
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

static int draw_qte(MonsterData monster) {
    draw_box(440, 160, 400, 400);
    DrawText(
        TextFormat(
            "Name: %s%s\nAge: %d\nWeakness: %s\nFavored Food: %s\n",
            title_to_string(monster.title),
            monster.name,
            monster.age,
            weakness_to_string(monster.weakness),
            food_to_string(monster.favored_food)
        ),
        450,
        180,
        20,
        BLACK
    );

    if (draw_button("Slay", 450, 460, 380, 40)) {
        return 1;
    }

    if (draw_button("Spare", 450, 510, 380, 40)) {
        return 2;
    }

    return 0;
}

DrawConfig make_draw_config(const int tile_size, const int tile_padding, const float entity_size) {
    DrawConfig config;
    config.tile_size    = tile_size;
    config.tile_padding = tile_padding;
    config.entity_size  = entity_size;
    return config;
}

Game load_game(const char *filename, const DrawConfig draw_config) {
    Game game;
    game.scene                           = load_scene(filename);
    game.draw_config                     = draw_config;
    game.ui_state.selected_building_type = TILE_GRASS;
    game.ui_state.selected_entity        = NULL;
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

void update_game(Game *game, float delta_time) {
    if (game->ui_state.selected_entity) {
        delta_time = 0.0f;
    }

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

    if (draw_button("Tower", 10, 40, 150, 40)) {
        game->ui_state.selected_building_type = TILE_TOWER;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse_pos = GetMousePosition();
        mouse_pos.x       += game->camera.target.x;
        mouse_pos.y       += game->camera.target.y;

        for (EntityId i = 0; i < game->scene.entity_pool.count; i++) {
            Entity *entity = &game->scene.entity_pool.entities[i];
            if (entity->type != ENTITY_MONSTER || entity->data.m.marked) {
                continue;
            }

            const float distance = Vector2Distance(mouse_pos, entity->position);
            if (distance < game->draw_config.entity_size) {
                game->ui_state.selected_entity = entity;
                break;
            }
        }
    }

    if (game->ui_state.selected_entity) {
        const int result = draw_qte(game->ui_state.selected_entity->data.m);
        if (result == 1) {
            game->ui_state.selected_entity->data.m.marked = true;
        }

        if (result != 0) {
            game->ui_state.selected_entity = NULL;
        }

        return;
    }

    const TileType building = game->ui_state.selected_building_type;
    if (building != TILE_GRASS && game->resources >= build_price(building)) {
        BeginMode2D(game->camera);

        const Vector2 mouse_pos = GetMousePosition();
        const int     x         = (int)(mouse_pos.x + game->camera.target.x) / game->draw_config.tile_size;
        const int     y         = (int)(mouse_pos.y + game->camera.target.y) / game->draw_config.tile_size;

        Color placeholder_color = GREEN;
        Tile *tile              = get_tile(&game->scene.grid, x, y);

        if (!tile || tile->type != TILE_GRASS) {
            placeholder_color = RED;
        }

        const int tile_size    = game->draw_config.tile_size;
        const int tile_padding = game->draw_config.tile_padding;

        if (building == TILE_TOWER) {
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
            if (tile && tile->type == TILE_GRASS) {
                game->resources -= build_price(building);
                tile->type      = building;
                if (building == TILE_TOWER) {
                    tile->data.tower.timer = 0.0f;
                }
            }
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            game->ui_state.selected_building_type = TILE_GRASS;
        }

        EndMode2D();
    } else {
        game->ui_state.selected_building_type = TILE_GRASS;
    }
}
