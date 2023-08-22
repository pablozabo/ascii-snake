#ifndef SCREEN_RESULT_H
#define SCREEN_RESULT_H

#include "../defs.h"

void screen_result_init(void);
void screen_result_dispose(void);
bool screen_result_is_completed(void);
void screen_result_update(void);
void screen_result_render(void);

#endif