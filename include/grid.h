#ifndef GAME_GRID_H
#define GAME_GRID_H

typedef enum {
    GRASS,
    TOWER,
    COLLECTOR,
    SPAWNER,
    BUILDING_TYPE_COUNT,
} TileType;

typedef struct {
    TileType type;
    int x;
    int y;
} Tile;

void draw_tile(const Tile* tile, int tile_size, int tile_padding);

typedef struct {
    int width;
    int height;
    Tile* tiles;
} Grid;

Grid make_grid(int width, int height);
void delete_grid(Grid* grid);

Tile* get_tile(Grid* grid, int x, int y);
void draw_grid(const Grid* grid, int tile_size, int tile_padding);

#endif //GAME_GRID_H
