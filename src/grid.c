#include "grid.h"

#include <raylib.h>

#include <stdlib.h>

int build_price(const TileType tile) {
    switch (tile) {
    case TILE_TOWER:
        return 5;
    case TILE_COLLECTOR:
        return 10;
    case TILE_SALT_CANNON:
        return 10;
    default:
        return 1023;
    }
}

SpawnQueue make_spawn_queue() {
    SpawnQueue queue;
    queue.ptr     = NULL;
    queue.size    = 0;
    queue.current = 0;
    return queue;
}

void delete_spawn_queue(SpawnQueue *queue) {
    free(queue->ptr);
}

void spawn_queue_push(SpawnQueue *queue, const MonsterType type) {
    if (queue->ptr) {
        queue->ptr              = realloc(queue->ptr, (queue->size + 1) * sizeof(MonsterType));
        queue->ptr[queue->size] = type;
        queue->size++;
    } else {
        queue->ptr     = malloc(sizeof(MonsterType));
        queue->ptr[0]  = type;
        queue->size    = 1;
        queue->current = 0;
    }
}

bool spawn_queue_increment(SpawnQueue *queue, MonsterType *out_type) {
    queue->current++;
    if (queue->current >= queue->size) {
        return false;
    }

    *out_type = queue->ptr[queue->current];
    return true;
}

void draw_tile(const SpriteStore *sprite_store, const Tile *tile, const int tile_size, const int tile_padding) {
    Texture2D sprite;
    const float scale = (float)tile_size / 16.0f;

    switch (tile->type) {
    case TILE_GRASS:
        sprite = sprite_store->grass;
        break;
    case TILE_PATH:
        sprite = sprite_store->path;
        break;
    case TILE_UP:
        sprite = sprite_store->path;
        break;
    case TILE_DOWN:
        sprite = sprite_store->path;
        break;
    case TILE_LEFT:
        sprite = sprite_store->path;
        break;
    case TILE_RIGHT:
        sprite = sprite_store->path;
        break;
    case TILE_CASTLE:
        sprite = sprite_store->castle;
        break;
    case TILE_TOWER:
        sprite = sprite_store->tower;
        break;
    case TILE_SALT_CANNON:
        sprite = sprite_store->cannon;
        break;
    case TILE_COLLECTOR:
        sprite = sprite_store->collector;
        break;
    case TILE_SPAWNER:
        sprite = sprite_store->path;
        break;
    default:
        sprite = sprite_store->grass;
        break;
    }

    DrawTextureEx(sprite_store->grass, (Vector2){.x = tile->x * tile_size, .y = tile->y * tile_size}, 0.0f, scale, WHITE);
    DrawTextureEx(sprite, (Vector2){.x = tile->x * tile_size, .y = tile->y * tile_size}, 0.0f, scale, WHITE);
}

Grid make_grid(const int width, const int height) {
    Grid grid;
    grid.width  = width;
    grid.height = height;
    grid.tiles  = (Tile *)malloc(width * height * sizeof(Tile));

    for (int i = 0; i < width * height; i++) {
        grid.tiles[i].type = TILE_GRASS;
        grid.tiles[i].x    = i % width;
        grid.tiles[i].y    = i / width;
    }

    return grid;
}

void delete_grid(Grid *grid) {
    for (int i = 0; i < grid->width; i++) {
        for (int j = 0; j < grid->height; j++) {
            Tile *tile = get_tile(grid, i, j);
            if (tile->type != TILE_SPAWNER) {
                continue;
            }

            for (int w = 0; w < tile->data.s.wave_count; w++) {
                delete_spawn_queue(&tile->data.s.waves[w]);
            }
        }
    }

    free(grid->tiles);
    grid->tiles = NULL;
}

Tile *get_tile(Grid *grid, const int x, const int y) {
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return NULL;
    }

    for (int i = 0; i < grid->width * grid->height; i++) {
        const Tile tile = grid->tiles[i];
        if (tile.x == x && tile.y == y) {
            return &grid->tiles[i];
        }
    }

    return NULL;
}

void draw_grid(const SpriteStore *sprite_store, const Grid *grid, const int tile_size, const int tile_padding) {
    for (int i = 0; i < grid->width * grid->height; i++) {
        const Tile tile = grid->tiles[i];
        draw_tile(sprite_store, &tile, tile_size, tile_padding);
    }
}
