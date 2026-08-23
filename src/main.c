#include "grid.h"
#include "entity.h"

#include <raylib.h>

int main() {
    Grid grid = make_grid(9, 9);
    EntityPool pool = make_entity_pool(1);
    InitWindow(1280, 720, "Game");

    get_tile(&grid, 4, 4)->type = TOWER;
    spawn_entity(&pool, (Vector2){.x = 96, .y = 96}, ENEMY);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_grid(&grid, 64, 4);
        draw_entities(&pool, 24);

        EndDrawing();
    }

    CloseWindow();
    delete_entity_pool(&pool);
    delete_grid(&grid);
    return 0;
}
