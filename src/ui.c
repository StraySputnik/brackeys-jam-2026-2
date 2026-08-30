#include "ui.h"

#include "rl.h"

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

int draw_qte(const MonsterData monster) {
    draw_box(440, 160, 400, 400);
    DrawText(
        TextFormat(
            "Name: %s%s\nAge: %d\nMonster Type: %s\nWeakness: %s\nStrength: %s\n",
            title_to_string(monster.title),
            monster.name,
            monster.age,
            monster_type_to_string(monster.monster_type),
            weakness_to_string(monster.weakness),
            strength_to_string(monster.strength)
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