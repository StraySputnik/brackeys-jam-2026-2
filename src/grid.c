#include "grid.h"

#include <raylib.h>

#include <stdlib.h>

int build_price(const TileType tile) {
    switch (tile) {
    case TILE_TOWER:
        return 5;
    default:
        return 1023;
    }
}

void draw_tile(const Tile *tile, const int tile_size, const int tile_padding) {
    Color color;

    switch (tile->type) {
    case TILE_GRASS:
        color = LIGHTGRAY;
        break;
    case TILE_PATH:
        color = GOLD;
        break;
    case TILE_UP:
        color = GOLD;
        break;
    case TILE_DOWN:
        color = GOLD;
        break;
    case TILE_LEFT:
        color = GOLD;
        break;
    case TILE_RIGHT:
        color = GOLD;
        break;
    case TILE_CASTLE:
        color = PURPLE;
        break;
    case TILE_TOWER:
        color = RED;
        break;
    case TILE_COLLECTOR:
        color = BLUE;
        break;
    case TILE_SPAWNER:
        color = GREEN;
        break;
    default:
        color = MAGENTA;
        break;
    }

    DrawRectangle(tile->x * tile_size + tile_padding, tile->y * tile_size + tile_padding,
                  tile_size - tile_padding * 2, tile_size - tile_padding * 2, color);
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

void draw_grid(const Grid *grid, const int tile_size, const int tile_padding) {
    for (int i = 0; i < grid->width * grid->height; i++) {
        const Tile tile = grid->tiles[i];
        draw_tile(&tile, tile_size, tile_padding);
    }
}
