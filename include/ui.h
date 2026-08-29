#ifndef GAME_UI_H
#define GAME_UI_H

#include "entity.h"

void draw_box(int x, int y, int width, int height);
int  draw_button(const char *label, int x, int y, int width, int height);
int  draw_qte(MonsterData monster);

#endif //GAME_UI_H
