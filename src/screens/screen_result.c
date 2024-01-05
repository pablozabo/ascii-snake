#include "screen_result.h"
#include "../common.h"

extern int		 g_key;
extern float32_t g_delta_time;
extern char		*g_asset_game_over;
extern score_t	 g_score;

static const uint8_t win_game_over_width   = 60;
static const uint8_t win_game_over_height  = 6;
static const uint8_t win_new_record_width  = 60;
static const uint8_t win_new_record_height = 3;
static const uint8_t win_play_again_width  = 60;
static const uint8_t win_play_again_height = 3;

static const uint8_t   game_over_char_width		 = 6;
static const uint8_t   game_over_char_height	 = 4;
static const uint8_t   game_over_length			 = 9;
static const float32_t game_over_animation_speed = 6;

static WINDOW *win_game_over;
static WINDOW *win_new_record;
static WINDOW *win_play_again;

static bool		 key_enter_pressed			= false;
static bool		 render_play_again_label	= true;
static float32_t elapsed_time				= 0;
static float32_t record_points				= 0;
static float32_t record_points_velocity		= 0;
static float32_t record_points_acceleration = 0.5;

static uint8_t game_over_current_char_index		= 0;
static uint8_t game_over_current_char_step		= 1;
static int8_t  game_over_current_char_direction = 0;

static void render_game_over(void);
static void render_new_record(void);
static void render_play_again(void);

void screen_result_init(void)
{
	uint8_t offset_y, offset_x;

	set_offset_yx(win_game_over_height + win_new_record_height + win_play_again_height, win_game_over_width, &offset_y, &offset_x);
	win_game_over = newwin(win_game_over_height, win_game_over_width, offset_y, offset_x);
	scrollok(win_game_over, TRUE);

	win_new_record = newwin(win_new_record_height, win_new_record_width, offset_y + win_game_over_height, offset_x);
	scrollok(win_new_record, TRUE);

	win_play_again = newwin(win_play_again_height, win_play_again_width, offset_y + win_game_over_height + win_new_record_height, offset_x);
	scrollok(win_play_again, TRUE);

	key_enter_pressed				 = false;
	elapsed_time					 = 0;
	game_over_current_char_index	 = 0;
	game_over_current_char_step		 = 1;
	game_over_current_char_direction = 0;
	record_points					 = 0;
	record_points_velocity			 = 1;
}

void screen_result_dispose(void)
{
	wclear(win_game_over);
	wrefresh(win_game_over);
	delwin(win_game_over);

	wclear(win_new_record);
	wrefresh(win_new_record);
	delwin(win_new_record);

	wclear(win_play_again);
	wrefresh(win_play_again);
	delwin(win_play_again);
}

bool screen_result_is_completed(void)
{
	return key_enter_pressed;
}

void screen_result_update(void)
{
	elapsed_time += g_delta_time;
	key_enter_pressed		= key_enter_pressed || g_key == CH_ENTER;
	render_play_again_label = (uint32_t)(elapsed_time) % 2;

	game_over_current_char_index	 = ((uint32_t)(elapsed_time * game_over_animation_speed)) % 9;
	game_over_current_char_direction = game_over_current_char_index % 2 ? -1 : 1;

	if (g_score.current >= g_score.record && record_points < g_score.current)
	{
		record_points_velocity += record_points_acceleration;
		record_points += record_points_velocity;

		if (record_points > g_score.current)
		{
			record_points = g_score.current;
		}
	}
}

void screen_result_render(void)
{

	render_game_over();

	if (g_score.current >= g_score.record && record_points <= g_score.current)
	{
		render_new_record();
	}

	render_play_again();
}

static void render_game_over(void)
{
	uint32_t i = 0;
	uint8_t
		ch,
		offset_x,
		offset_y,
		wx,
		wy,
		char_index = 0;

	wclear(win_game_over);
	wattron(win_game_over, COLOR_PAIR(COLOR_PAIR_RED));

	offset_x = (win_game_over_width - 55) * 0.5;
	offset_y = (win_game_over_height - 4) * 0.5;

	while (char_index < game_over_length)
	{
		for (uint8_t y = 0; y < game_over_char_height; y++)
		{
			for (uint8_t x = 0; x < game_over_char_width; x++)
			{
				i  = (char_index * game_over_char_width) + ((game_over_length * game_over_char_width) + 1) * y + x;
				ch = g_asset_game_over[i];
				wx = offset_x + game_over_char_width * char_index + x;
				wy = offset_y + y;

				if (game_over_current_char_index == char_index)
				{
					wy += game_over_current_char_step * game_over_current_char_direction;
				}

				mvwprintw(win_game_over, wy, wx, "%c", ch);
			}
		}

		char_index++;
	}

	wattroff(win_game_over, COLOR_PAIR(COLOR_PAIR_RED));
	wrefresh(win_game_over);

	wrefresh(win_game_over);
}

static void render_new_record(void)
{
	uint8_t offset_x;
	char	record[30] = { '\0' };
	wclear(win_new_record);

	sprintf(record, "New record! %d", (uint32_t)record_points);
	offset_x = (win_new_record_width - 12) * 0.5;

	wattron(win_new_record, COLOR_PAIR(COLOR_PAIR_GREEN));
	mvwprintw(win_new_record, 1, offset_x, "%s", record);
	wattroff(win_new_record, COLOR_PAIR(COLOR_PAIR_GREEN));

	wrefresh(win_new_record);
}

static void render_play_again(void)
{
	uint8_t offset_x;
	wclear(win_play_again);

	if (render_play_again_label)
	{
		offset_x = (win_play_again_width - 25) * 0.5;
		mvwprintw(win_play_again, 2, offset_x, "Press enter to play again");
	}

	wrefresh(win_play_again);
}