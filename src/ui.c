#include "ui.h"

#include <raylib.h>

void draw_box(const int x, const int y, const int width, const int height) {
    const Rectangle button_rect = {.x = x, .y = y, .width = width, .height = height};
    const Rectangle border_rect = {.x = x - 2, .y = y - 2, .width = width + 4, .height = height + 4};

    DrawRectangleRec(border_rect, DARKGRAY);
    DrawRectangleRec(button_rect, GRAY);
}

int draw_button(const char *label, const int x, const int y, const int width, const int height) {
    const Rectangle button_rect = {.x = x, .y = y, .width = width, .height = height};
    const Rectangle border_rect = {.x = x - 2, .y = y - 2, .width = width + 4, .height = height + 4};

    Color       button_color = LIGHTGRAY;
    const Color text_color   = BLACK;

    int out = 0;
    if (CheckCollisionPointRec(GetMousePosition(), button_rect)) {
        button_color = GRAY;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            button_color = RAYWHITE;
            out          = 1;
        }
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            out = 2;
        }
    }

    DrawRectangleRec(border_rect, DARKGRAY);
    DrawRectangleRec(button_rect, button_color);
    DrawText(label, x + 10, y + height / 4, 20, text_color);

    return out;
}