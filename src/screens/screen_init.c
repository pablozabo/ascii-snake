#include "screen_init.h"
#include "../common.h"

extern char			  *g_asset_splash;
extern int			   g_key;
extern const float32_t g_target_frame_time;

#define ASSET_SPLASH_SNAKE_ROWS 9

static const char	*label_start		= "Press ENTER to start";
static const uint8_t win_splash_width	= 46;
static const uint8_t win_splash_height	= 17;
static const uint8_t win_actions_width	= 20;
static const uint8_t win_actions_height = 2;

static WINDOW  *win_splash;
static WINDOW  *win_actions;
static bool		print_label_start = true;
static bool		key_enter_pressed = false;
static uint32_t elapsed_time	  = 0;

static void render_splash(void);

void screen_init_init(void)
{
	uint8_t offset_y, offset_y2, offset_x;

	set_offset_yx(win_splash_height, win_splash_width, &offset_y, &offset_x);
	win_splash = newwin(win_splash_height, win_splash_width, offset_y, offset_x);
	scrollok(win_splash, TRUE);

	set_offset_yx(win_actions_height, win_actions_width, &offset_y2, &offset_x);
	win_actions = newwin(win_actions_height, win_actions_width, offset_y + win_splash_height, offset_x);
	scrollok(win_actions, TRUE);

	render_splash();
}

void screen_init_dispose(void)
{
	wclear(win_splash);
	wrefresh(win_splash);
	delwin(win_splash);

	wclear(win_actions);
	wrefresh(win_actions);
	delwin(win_actions);
}

bool screen_init_is_completed(void)
{
	return key_enter_pressed;
}

void screen_init_update(void)
{
	elapsed_time += g_target_frame_time;
	key_enter_pressed = key_enter_pressed || g_key == CH_ENTER;
	print_label_start = !key_enter_pressed && (uint32_t)(QUARTER_SECONDS(elapsed_time)) % 2;
}

void screen_init_render(void)
{
	wclear(win_actions);

	if (print_label_start)
	{
		mvwprintw(win_actions, 0, 0, "%s", label_start);
	}

	wrefresh(win_actions);

	if (g_key == KEY_RESIZE)
	{
		render_splash();
	}
}

static void render_splash(void)
{
	uint32_t i = 0;
	uint8_t	 ch,
		x = 0,
		y = 0;

	wclear(win_splash);
	wattron(win_splash, COLOR_PAIR(COLOR_PAIR_GREEN));

	while ((ch = g_asset_splash[i++]) != CH_EOS)
	{
		if (ch == CH_EOL)
		{
			x = 0;
			y++;
			continue;
		}

		if (y == ASSET_SPLASH_SNAKE_ROWS)
		{
			wattroff(win_splash, COLOR_PAIR(COLOR_PAIR_GREEN));
			wattron(win_splash, COLOR_PAIR(COLOR_PAIR_RED));
		}

		mvwprintw(win_splash, y, x++, "%c", ch);
	}

	wattroff(win_splash, COLOR_PAIR(COLOR_PAIR_RED));
	wrefresh(win_splash);
}