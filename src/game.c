#include "game.h"
#include "ui.h"

#include "raymath.h"

#include <stdio.h>

#define INITIAL_CASTLE_HEALTH       20
#define INITIAL_COINS               15
#define ENEMY_SPEED                 1
#define PROJECTILE_SPEED            16
#define TOWER_ATTACK_INTERVAL       3
#define TOWER_RANGE                 3
#define SALT_CANNON_ATTACK_INTERVAL 4
#define SALT_CANNON_RANGE           4
#define SPAWN_INTERVAL              2
#define COLLECT_INTERVAL            5
#define PAUSE_COOLDOWN              1.5

static float pause_times[] = {
    5.0f,
    4.0f,
    3.0f,
    2.5f,
    2.0f,
    1.5f,
};

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

static bool all_spawners_finished(Game *game) {
    for (int i = 0; i < game->scene.grid.width; i++) {
        for (int j = 0; j < game->scene.grid.height; j++) {
            Tile *tile = get_tile(&game->scene.grid, i, j);
            if (tile->type != TILE_SPAWNER) {
                continue;
            }

            if (!tile->data.s.finished) {
                return false;
            }
        }
    }

    return true;
}

static bool all_entities_dead(const Game *game) {
    for (EntityId i = 0; i < game->scene.entity_pool.count; i++) {
        if (game->scene.entity_pool.entities[i].type != ENTITY_NULL) {
            return false;
        }
    }

    return true;
}

static bool wave_finished(Game *game) {
    return game->current_wave < WAVE_COUNT - 1 && all_spawners_finished(game) && all_entities_dead(game);
}

static void next_wave(Game *game) {
    game->current_wave++;
    if (game->current_wave >= WAVE_COUNT) {
        game->current_wave--;
        return;
    }

    for (int i = 0; i < game->scene.grid.width; i++) {
        for (int j = 0; j < game->scene.grid.height; j++) {
            Tile *tile = get_tile(&game->scene.grid, i, j);
            if (tile->type != TILE_SPAWNER) {
                continue;
            }

            tile->data.s.finished = false;
        }
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
                game->coins += 1;
            } else {
                game->castle_health -= 1;
                play_sfx(&game->audio_store, "CastleDamage");
            }

            despawn_entity(&game->scene.entity_pool, entity->id);
            return;
        }

        MonsterData data = entity->data.m;
        data.direction.x *= 0.02f * (float)game->draw_config.tile_size;
        data.direction.y *= 0.02f * (float)game->draw_config.tile_size;

        const bool within_tile = Vector2Distance(entity->position, tile_pos) <=
            0.02f * (float)game->draw_config.tile_size;
        const bool bypassed_tile = Vector2Distance(Vector2Subtract(entity->position, data.direction), tile_pos) <=
            0.02f * (float)game->draw_config.tile_size;

        if (!within_tile && !bypassed_tile) {
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
        {
            entity->position.x +=
                entity->data.p.direction.x * PROJECTILE_SPEED * (float)game->draw_config.tile_size * delta_time;
            entity->position.y +=
                entity->data.p.direction.y * PROJECTILE_SPEED * (float)game->draw_config.tile_size * delta_time;

            if (entity->position.x < -1000 || entity->position.x > 5000 || entity->position.y < -1000 || entity->
                position.y
                > 5000) {
                despawn_entity(&game->scene.entity_pool, entity->id);
                break;
            }

            Entity *target = get_closest_target(game, entity->position);
            if (!target) {
                break;
            }

            const MonsterType target_type = target->data.m.monster_type;
            if (Vector2Distance(entity->position, target->position) < game->draw_config.entity_size) {
                switch (target_type) {
                case MONSTER_VAMPIRE:
                    play_sfx(&game->audio_store, "VampireHit");
                    break;
                case MONSTER_WEREWOLF:
                    play_sfx(&game->audio_store, "WerewolfHit");
                    break;
                case MONSTER_GHOST:
                    play_sfx(&game->audio_store, "GhostHit");
                    break;
                default:
                    break;
                }

                target->data.m.health--;
                if (target->data.m.health <= 0) {
                    if (is_friendly(target->data.m)) {
                        play_sfx(&game->audio_store, "WrongEnemyKilled");
                    } else {
                        play_sfx(&game->audio_store, "EnemyDeath");
                    }

                    despawn_entity(&game->scene.entity_pool, target->id);
                }

                despawn_entity(&game->scene.entity_pool, entity->id);
                break;
            }

            break;
        }
    case ENTITY_CANNONBALL:
        {
            entity->position.x +=
                entity->data.p.direction.x * PROJECTILE_SPEED * (float)game->draw_config.tile_size * delta_time;
            entity->position.y +=
                entity->data.p.direction.y * PROJECTILE_SPEED * (float)game->draw_config.tile_size * delta_time;

            if (entity->position.x < -1000 || entity->position.x > 5000 || entity->position.y < -1000 || entity->
                position.y
                > 5000) {
                despawn_entity(&game->scene.entity_pool, entity->id);
                break;
            }

            Entity *target = get_closest_target(game, entity->position);
            if (!target) {
                break;
            }

            const MonsterType target_type = target->data.m.monster_type;
            if (Vector2Distance(entity->position, target->position) < game->draw_config.entity_size) {
                switch (target_type) {
                case MONSTER_VAMPIRE:
                    play_sfx(&game->audio_store, "VampireHit");
                    break;
                case MONSTER_WEREWOLF:
                    play_sfx(&game->audio_store, "WerewolfHit");
                    break;
                case MONSTER_GHOST:
                    play_sfx(&game->audio_store, "GhostHit");
                    target->data.m.health -= 2;
                    break;
                default:
                    break;
                }

                target->data.m.health--;
                if (target->data.m.health <= 0) {
                    if (is_friendly(target->data.m)) {
                        play_sfx(&game->audio_store, "WrongEnemyKilled");
                    } else {
                        play_sfx(&game->audio_store, "EnemyDeath");
                    }

                    despawn_entity(&game->scene.entity_pool, target->id);
                }

                despawn_entity(&game->scene.entity_pool, entity->id);
                break;
            }

            break;
        }
    default:
        break;
    }
}

static void update_tile(Tile *tile, Game *game, const float delta_time) {
    switch (tile->type) {
    case TILE_TOWER:
        {
            TowerData *data = &tile->data.t;
            data->timer     += delta_time;

            if (data->timer >= TOWER_ATTACK_INTERVAL) {
                const Vector2 position = {
                    .x = ((float)tile->x + 0.5f) * (float)game->draw_config.tile_size,
                    .y = ((float)tile->y + 0.5f) * (float)game->draw_config.tile_size,
                };

                Entity *target = get_closest_target(game, position);
                if (!target) {
                    break;
                }

                if (target->data.m.target_count >= target->data.m.health) {
                    break;
                }

                if (Vector2Distance(position, target->position) > TOWER_RANGE * (float)game->draw_config.tile_size) {
                    break;
                }

                target->data.m.target_count++;
                data->timer = 0.0f;
                play_sfx(&game->audio_store, "TowerShoot");

                Vector2 direction = Vector2Subtract(target->position, position);
                direction         = Vector2Normalize(direction);

                const EntityId id = spawn_entity(&game->scene.entity_pool, position, ENTITY_PROJECTILE);
                get_entity(&game->scene.entity_pool, id)->data.p.direction = direction;
            }

            break;
        }
    case TILE_SALT_CANNON:
        {
            SaltCannonData *data = &tile->data.sc;
            data->timer          += delta_time;

            if (data->timer >= SALT_CANNON_ATTACK_INTERVAL) {
                const Vector2 position = {
                    .x = ((float)tile->x + 0.5f) * (float)game->draw_config.tile_size,
                    .y = ((float)tile->y + 0.5f) * (float)game->draw_config.tile_size,
                };

                Entity *target = get_closest_target(game, position);
                if (!target) {
                    break;
                }

                if (target->data.m.target_count >= target->data.m.health) {
                    break;
                }

                if (Vector2Distance(position, target->position) >
                    SALT_CANNON_RANGE * (float)game->draw_config.tile_size) {
                    break;
                }

                target->data.m.target_count++;
                data->timer = 0.0f;
                play_sfx(&game->audio_store, "TowerShoot");

                Vector2 direction = Vector2Subtract(target->position, position);
                direction         = Vector2Normalize(direction);

                const EntityId id = spawn_entity(&game->scene.entity_pool, position, ENTITY_PROJECTILE);
                get_entity(&game->scene.entity_pool, id)->data.p.direction = direction;
            }

            break;
        }
    case TILE_SPAWNER:
        {
            SpawnerData *data = &tile->data.s;
            data->timer       += delta_time;

            if (data->timer >= SPAWN_INTERVAL) {
                MonsterType type;
                if (!spawn_queue_increment(&data->waves[game->current_wave], &type)) {
                    data->finished = true;
                    break;
                }

                data->timer            = 0.0f;
                const Vector2 position = {
                    .x = ((float)tile->x + 0.5f) * (float)game->draw_config.tile_size,
                    .y = ((float)tile->y + 0.5f) * (float)game->draw_config.tile_size,
                };

                if (type == MONSTER_NONE) {
                    break;
                }

                const EntityId id = spawn_entity(&game->scene.entity_pool, position, ENTITY_MONSTER);
                MonsterData *  m  = &get_entity(&game->scene.entity_pool, id)->data.m;

                m->direction    = data->direction;
                m->monster_type = type;
                m->marked       = false;
                m->health       = get_health_for_monster(type);
                m->target_count = 0;

                bool friendly = false;
                if (data->friendly_count > 0) {
                    friendly = GetRandomValue(0, 1);
                    if (friendly) {
                        data->friendly_count--;
                    }
                }

                randomize_monster_data(m, m->monster_type, friendly);
            }

            break;
        }
    case TILE_COLLECTOR:
        {
            CollectorData *data = &tile->data.c;
            data->timer         += delta_time;

            if (wave_finished(game)) {
                data->timer = 0.0f;
            }

            if (data->timer >= COLLECT_INTERVAL) {
                data->timer = 0.0f;
                game->coins += 1;
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

static void handle_title_ui(Game *game) {
    DrawText("KNIGHT GUARD", 370, 120, 72, WHITE);
    if (draw_button("Start", 500, 240, 280, 40)) {
        play_sfx(&game->audio_store, "ClickMenuButton");
        game->scene_idx = 1;
    }

    if (draw_button("Quit", 500, 300, 280, 40)) {
        play_sfx(&game->audio_store, "ClickMenuButton");
        game->scene_idx = -1;
    }
}

static void handle_lose_ui(Game *game) {
    DrawText("YOU LOST!", 450, 120, 72, RED);
    if (draw_button("Restart", 500, 240, 280, 40)) {
        game->ui_state.selected_building_type = TILE_GRASS;
        game->ui_state.selected_entity        = NULL;
        game->castle_health                   = INITIAL_CASTLE_HEALTH;
        game->coins                           = INITIAL_COINS;
        game->current_wave                    = 0;
        game->pause_timer                     = 0.0f;
        game->scene                           = load_scene(game->filename);

        play_sfx(&game->audio_store, "ClickMenuButton");
        game->scene_idx = 1;
    }

    if (draw_button("Quit", 500, 300, 280, 40)) {
        play_sfx(&game->audio_store, "ClickMenuButton");
        game->scene_idx = -1;
    }
}

static void handle_win_ui(Game *game) {
    DrawText("YOU WON!", 470, 120, 72, GOLD);

    DrawText("-- Credits --", 535, 220, 32, WHITE);
    DrawText("Code - StraySputnik", 480, 320, 32, WHITE);
    DrawText("Music & SFX - Simulacrum + Itroma", 380, 360, 32, WHITE);

    if (draw_button("Quit", 500, 500, 280, 40)) {
        play_sfx(&game->audio_store, "ClickMenuButton");
        game->scene_idx = -1;
    }
}

static void handle_ingame_ui(Game *game) {
    DrawText(TextFormat("HP: %d", game->castle_health), 10, 10, 20, RED);
    DrawText(TextFormat("Coins: %d", game->coins), 80, 10, 20, YELLOW);
    DrawText(TextFormat("Wave %d/%d", game->current_wave + 1, WAVE_COUNT), 590, 10, 20, RAYWHITE);

    if (game->pause_timer > 0.0f) {
        DrawRectangle(0, 710, (int)(1280 * game->pause_timer / pause_times[game->current_wave]), 10, WHITE);
    } else {
        DrawRectangle(0, 710, (int)(1280 * -game->pause_timer / PAUSE_COOLDOWN), 10, BLUE);
    }

    if (wave_finished(game)) {
        if (draw_button("Next Wave", 1120, 40, 150, 40)) {
            play_sfx(&game->audio_store, "ClickMenuButton");
            play_sfx(&game->audio_store, "WaveStart");
            game->castle_health = INITIAL_CASTLE_HEALTH;
            next_wave(game);
        }

        for (int i = 0; i < game->scene.grid.width; i++) {
            for (int j = 0; j < game->scene.grid.height; j++) {
                const Tile *tile = get_tile(&game->scene.grid, i, j);
                if (tile->type != TILE_SPAWNER) {
                    continue;
                }

                const int x = (int)((i + 0.25) * game->draw_config.tile_size - game->camera.target.x);
                const int y = (int)((j + 0.25) * game->draw_config.tile_size - game->camera.target.y);

                int count = 0;
                for (int e = 0; e < tile->data.s.waves[game->current_wave + 1].size; e++) {
                    if (tile->data.s.waves[game->current_wave + 1].ptr[e] != MONSTER_NONE) {
                        count++;
                    }
                }

                DrawText(TextFormat("%i", count), x, y, 32, RED);
            }
        }
    }

    if (draw_button("Tower (5)", 10, 40, 200, 40)) {
        play_sfx(&game->audio_store, "ClickMenuButton");
        game->ui_state.selected_building_type = TILE_TOWER;
    }

    if (draw_button("Salt Cannon (10)", 10, 90, 200, 40)) {
        play_sfx(&game->audio_store, "ClickMenuButton");
        game->ui_state.selected_building_type = TILE_SALT_CANNON;
    }

    if (draw_button("Collector (10)", 10, 140, 200, 40)) {
        play_sfx(&game->audio_store, "ClickMenuButton");
        game->ui_state.selected_building_type = TILE_COLLECTOR;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && game->pause_timer < -PAUSE_COOLDOWN) {
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
                play_sfx(&game->audio_store, "OpenInfoPage");

                game->pause_timer = pause_times[game->current_wave];
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
            play_sfx(&game->audio_store, "CloseInfoPage");
            game->ui_state.selected_entity = NULL;
            game->pause_timer              = 0.0f;
        }

        return;
    }

    const TileType building = game->ui_state.selected_building_type;
    if (building != TILE_GRASS && game->coins >= build_price(building)) {
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

        if (building == TILE_SALT_CANNON) {
            DrawCircle(
                x * tile_size + tile_size / 2,
                y * tile_size + tile_size / 2,
                SALT_CANNON_RANGE * (float)tile_size,
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
                game->coins -= build_price(building);
                tile->type  = building;
                play_sfx(&game->audio_store, "TowerBuild");

                if (building == TILE_TOWER) {
                    tile->data.t.timer = 0.0f;
                }

                if (building == TILE_SALT_CANNON) {
                    tile->data.sc.timer = 0.0f;
                }

                if (building == TILE_COLLECTOR) {
                    tile->data.c.timer = 0.0f;
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

DrawConfig make_draw_config(const int tile_size, const int tile_padding, const float entity_size) {
    DrawConfig config;
    config.tile_size    = tile_size;
    config.tile_padding = tile_padding;
    config.entity_size  = entity_size;
    return config;
}

Game load_game(const char *filename, const DrawConfig draw_config) {
    Game game;
    game.filename                        = filename;
    game.scene                           = load_scene(filename);
    game.draw_config                     = draw_config;
    game.ui_state.selected_building_type = TILE_GRASS;
    game.ui_state.selected_entity        = NULL;
    game.audio_store                     = make_audio_store();

    audio_store_load(&game.audio_store, "res/sfx/CastleDamaged.wav", "CastleDamage", 1.0f);
    audio_store_load(&game.audio_store, "res/sfx/ClickMenuButton.wav", "ClickMenuButton", 1.0f);
    audio_store_load(&game.audio_store, "res/sfx/CloseInfoPage.wav", "CloseInfoPage", 3.0f);
    audio_store_load(&game.audio_store, "res/sfx/EnemyDies.wav", "EnemyDeath", 3.0f);
    audio_store_load(&game.audio_store, "res/sfx/GenericTowerBuild.wav", "TowerBuild", 1.0f);
    audio_store_load(&game.audio_store, "res/sfx/GenericTowerShoot.wav", "TowerShoot", 1.0f);
    audio_store_load(&game.audio_store, "res/sfx/GhostHit.wav", "GhostHit", 1.0f);
    audio_store_load(&game.audio_store, "res/sfx/OpenInfoPage.wav", "OpenInfoPage", 3.0f);
    audio_store_load(&game.audio_store, "res/sfx/VampireHit.wav", "VampireHit", 1.0f);
    audio_store_load(&game.audio_store, "res/sfx/WaveStart.wav", "WaveStart", 1.0f);
    audio_store_load(&game.audio_store, "res/sfx/WerewolfHit.wav", "WerewolfHit", 1.0f);
    audio_store_load(&game.audio_store, "res/sfx/WrongEnemyKilled.wav", "WrongEnemyKilled", 1.0f);

    game.castle_health = INITIAL_CASTLE_HEALTH;
    game.coins         = INITIAL_COINS;
    game.current_wave  = 0;
    game.pause_timer   = 0.0f;
    game.scene_idx     = 0;

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
    delete_audio_store(&game->audio_store);
}

int update_game(Game *game, float delta_time) {
    if (game->current_wave == WAVE_COUNT) {
        game->scene_idx = 3;
    }

    if (game->castle_health <= 0) {
        game->scene_idx = 2;
    }

    if (IsKeyPressed(KEY_E)) {
        game->scene_idx++;
    }

    if (game->scene_idx != 1) {
        return game->scene_idx;
    }

    game->pause_timer -= delta_time;
    if (game->pause_timer <= 0.0f) {
        game->ui_state.selected_entity = NULL;
    }

    if (game->ui_state.selected_entity) {
        delta_time = 0.0f;
    }

    update_tiles(game, delta_time);
    update_entities(game, delta_time);

    return game->scene_idx;
}

void draw_game(const Game *game) {
    if (game->scene_idx != 1) {
        return;
    }

    BeginMode2D(game->camera);
    draw_scene(
        &game->scene, game->draw_config.tile_size,
        game->draw_config.tile_padding,
        game->draw_config.entity_size
    );
    EndMode2D();
}

void handle_ui(Game *game) {
    switch (game->scene_idx) {
    case 0:
        handle_title_ui(game);
        break;
    case 1:
        handle_ingame_ui(game);
        break;
    case 2:
        handle_lose_ui(game);
        break;
    case 3:
        handle_win_ui(game);
        break;
    default:
        break;
    }
}
