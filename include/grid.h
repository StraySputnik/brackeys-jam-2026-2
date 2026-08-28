#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "entity.h"

typedef enum {
    TILE_GRASS,
    TILE_PATH,
    TILE_UP,
    TILE_DOWN,
    TILE_LEFT,
    TILE_RIGHT,
    TILE_CASTLE,
    TILE_TOWER,
    TILE_COLLECTOR,
    TILE_SPAWNER,
    TILE_TYPE_COUNT,
} TileType;

int build_price(TileType tile);

typedef struct {
    float timer;
} TowerData;

typedef struct {
    MonsterType *ptr;
    size_t       size;
    size_t       current;
} SpawnQueue;

SpawnQueue make_spawn_queue();
void       delete_spawn_queue(SpawnQueue *queue);

void spawn_queue_push(SpawnQueue *queue, MonsterType type);
bool spawn_queue_increment(SpawnQueue *queue, MonsterType *out_type);

typedef struct {
    Vector2     direction;
    float       timer;
    SpawnQueue *waves;
    size_t      wave_count;
    bool        finished;
} SpawnerData;

typedef struct {
    TileType type;
    int      x;
    int      y;

    union {
        TowerData   t;
        SpawnerData s;
    } data;
} Tile;

void draw_tile(const Tile *tile, int tile_size, int tile_padding);

typedef struct {
    int   width;
    int   height;
    Tile *tiles;
} Grid;

Grid make_grid(int width, int height);
void delete_grid(Grid *grid);

Tile *get_tile(Grid *grid, int x, int y);
void  draw_grid(const Grid *grid, int tile_size, int tile_padding);

#endif //GAME_GRID_H
