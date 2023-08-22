#include "screen_result.h"
#include "../common.h"

extern int			   g_key;
extern const float32_t g_target_frame_time;

static const uint8_t win_result_width  = 60;
static const uint8_t win_result_height = 3;

static WINDOW  *win_result;
static bool		key_enter_pressed = false;
static bool		print_label_start = true;
static uint32_t elapsed_time	  = 0;

void screen_result_init(void)
{
	uint8_t offset_y, offset_x;
	set_offset_yx(win_result_height, win_result_width, &offset_y, &offset_x);
	win_result = newwin(win_result_height, win_result_width, offset_y, offset_x);
	scrollok(win_result, TRUE);
	key_enter_pressed = false;
	elapsed_time	  = 0;
}

void screen_result_dispose(void)
{
	wclear(win_result);
	wrefresh(win_result);
	delwin(win_result);
}

bool screen_result_is_completed(void)
{
	return key_enter_pressed;
}

void screen_result_update(void)
{
	elapsed_time += g_target_frame_time;
	key_enter_pressed = key_enter_pressed || g_key == CH_ENTER;
	print_label_start = (uint32_t)(QUARTER_SECONDS(elapsed_time)) % 2;
}

void screen_result_render(void)
{
	uint8_t offset_x;
	wclear(win_result);

	if (print_label_start)
	{
		offset_x = (win_result_width - 25) * 0.5;
		mvwprintw(win_result, 2, offset_x, "Press enter to play again");
	}

	wrefresh(win_result);
}
