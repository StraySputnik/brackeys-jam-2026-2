#include "grid.h"

#include <raylib.h>

#include <stdlib.h>

void draw_tile(const Tile* tile, const int tile_size, const int tile_padding) {
    Color color;

    switch (tile->type) {
    case GRASS:
        color = LIGHTGRAY;
        break;
    case PATH:
        color = GOLD;
        break;
    case UP:
        color = GOLD;
        break;
    case DOWN:
        color = GOLD;
        break;
    case LEFT:
        color = GOLD;
        break;
    case RIGHT:
        color = GOLD;
        break;
    case CASTLE:
        color = PURPLE;
        break;
    case TOWER:
        color = RED;
        break;
    case COLLECTOR:
        color = BLUE;
        break;
    case SPAWNER:
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
    grid.width = width;
    grid.height = height;
    grid.tiles = (Tile*)malloc(width * height * sizeof(Tile));

    for (int i = 0; i < width * height; i++) {
        grid.tiles[i].type = GRASS;
        grid.tiles[i].x = i % width;
        grid.tiles[i].y = i / width;
    }

    return grid;
}

void delete_grid(Grid* grid) {
    free(grid->tiles);
    grid->tiles = NULL;
}

Tile* get_tile(Grid* grid, const int x, const int y) {
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

void draw_grid(const Grid* grid, const int tile_size, const int tile_padding) {
    for (int i = 0; i < grid->width * grid->height; i++) {
        const Tile tile = grid->tiles[i];
        draw_tile(&tile, tile_size, tile_padding);
    }
}