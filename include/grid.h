#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "entity.h"

typedef enum {
    GRASS,
    PATH,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CASTLE,
    TOWER,
    COLLECTOR,
    SPAWNER,
    BUILDING_TYPE_COUNT,
} TileType;

typedef struct {
    float timer;
} TowerData;

typedef struct {
    Vector2    direction;
    float      timer;
    float      spawn_interval;
    EntityType entity_type;
} SpawnerData;

typedef struct {
    TileType type;
    int      x;
    int      y;

    union {
        TowerData   tower;
        SpawnerData spawner;
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
