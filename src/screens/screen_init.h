#ifndef SCREEN_INIT_H
#define SCREEN_INIT_H

#include "../defs.h"

void screen_init_init(void);
void screen_init_dispose(void);
bool screen_init_is_completed(void);
void screen_init_update(void);
void screen_init_render(void);

#endif