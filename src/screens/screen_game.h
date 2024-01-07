#ifndef SCREEN_GAME_H
#define SCREEN_GAME_H

#include "../defs.h"

void screen_game_init(void);
void screen_game_dispose(void);
bool screen_game_is_completed(void);
void screen_game_update(void);
void screen_game_render(void);
void screen_game_window_resized(void);

#endif