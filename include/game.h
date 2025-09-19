#ifndef GAME_H
#define GAME_H

#include "../include/input.h"
#include "../include/view.h"
#include "../include/char.h"

void game_init(void);
void game_start(void);
double game_update(void);
void game_deinit(void);

#endif
