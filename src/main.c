#include "common.h"
#include "defs.h"
#include "screens/screens.h"

#define TERMINAL_COLS 100
#define TERMINAL_ROWS 50

#define FILE_SPLASH "assets/splash.txt"
#define FILE_GAME_OVER "assets/game_over.txt"

typedef enum screen_t
{
	SCREEN_INIT	  = 1,
	SCREEN_GAME	  = 2,
	SCREEN_RESULT = 3
} screen_t;

// #GLOBAL VARIABLES
bool			g_running = true;
int				g_key;
const float32_t g_target_frame_time = 1000 / 30; // 30 FPS
char		   *g_asset_splash		= NULL;
char		   *g_asset_game_over	= NULL;
score_t			g_score				= { .current = 0 };

static screen_action_t		 screen_action_init			  = NULL;
static screen_action_t		 screen_action_dispose		  = NULL;
static screen_action_t		 screen_action_update		  = NULL;
static screen_action_t		 screen_action_render		  = NULL;
static screen_is_completed_t screen_is_completed		  = NULL;
static screen_action_t		 screen_action_window_resized = NULL;
static screen_t				 current_screen				  = 0;

static float32_t last_update_time = 0.0;

static void init(void);
static void dispose(void);
static void load_assets(void);
static void load_asset(const char *file, char **dest);
static void load_score(void);
static void update_state(void);
static void loop(void);

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	init();
	loop();
	dispose();

	return 0;
}

static void init(void)
{
	load_assets();
	load_score();
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	timeout(5);
	resize_term(TERMINAL_ROWS, TERMINAL_COLS);
	start_color();

	init_pair(COLOR_PAIR_BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(COLOR_PAIR_BLUE_BK, COLOR_WHITE, COLOR_BLUE);
	init_pair(COLOR_PAIR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_PAIR_RED_BK, COLOR_WHITE, COLOR_RED);
	init_pair(COLOR_PAIR_GREEN, COLOR_GREEN, COLOR_BLACK);

	init_color(COLOR_PAIR_RED, 1000, 0, 0);
	init_color(COLOR_PAIR_GREEN, 0, 700, 0);
	init_color(COLOR_PAIR_BLUE, 0, 0, 700);

	refresh();
}

static void dispose(void)
{
	if (g_asset_splash)
	{
		free(g_asset_splash);
	}

	if (g_asset_game_over)
	{
		free(g_asset_game_over);
	}

	use_default_colors();
	endwin();
}

static void loop(void)
{
	while (g_running)
	{
		g_key = getch();

		if (g_key == KEY_F(1) || g_key == CH_ESC)
		{
			break;
		}
		else if (g_key == KEY_RESIZE)
		{
			resize_term(TERMINAL_ROWS, TERMINAL_COLS);
			noecho();
			cbreak();
			curs_set(0);
			refresh();

			if (screen_action_window_resized)
			{
				screen_action_window_resized();
			}
		}

		float32_t real_delta_time = CURRENT_TIME - last_update_time;
		last_update_time += real_delta_time;

		if (real_delta_time < g_target_frame_time)
		{
			custom_delay(g_target_frame_time - real_delta_time);
		}

		update_state();
		screen_action_update();
		screen_action_render();
	}

	if (screen_action_dispose)
	{
		screen_action_dispose();
	}
}

static void update_state(void)
{
	if (!current_screen)
	{
		screen_action_init			 = &screen_init_init;
		screen_action_dispose		 = &screen_init_dispose;
		screen_action_update		 = &screen_init_update;
		screen_action_render		 = &screen_init_render;
		screen_is_completed			 = &screen_init_is_completed;
		screen_action_window_resized = &screen_init_window_resized;
		screen_action_init();
		current_screen = SCREEN_INIT;
	}
	else if ((current_screen == SCREEN_INIT || current_screen == SCREEN_RESULT) && screen_is_completed())
	{
		screen_action_dispose();
		screen_action_init			 = &screen_game_init;
		screen_action_dispose		 = &screen_game_dispose;
		screen_action_update		 = &screen_game_update;
		screen_action_render		 = &screen_game_render;
		screen_is_completed			 = &screen_game_is_completed;
		screen_action_window_resized = NULL;
		screen_action_init();
		current_screen = SCREEN_GAME;
	}
	else if (current_screen == SCREEN_GAME && screen_is_completed())
	{
		screen_action_dispose();
		screen_action_init			 = &screen_result_init;
		screen_action_dispose		 = &screen_result_dispose;
		screen_action_update		 = &screen_result_update;
		screen_action_render		 = &screen_result_render;
		screen_is_completed			 = &screen_result_is_completed;
		screen_action_window_resized = NULL;
		screen_action_init();
		current_screen = SCREEN_RESULT;
	}
}

static void load_assets(void)
{
	load_asset(FILE_SPLASH, &g_asset_splash);
	load_asset(FILE_GAME_OVER, &g_asset_game_over);
}

static void load_asset(const char *file, char **dest)
{
	FILE *f = fopen(file, "r");
	ASSERT(f);

	fseek(f, 0, SEEK_END);
	int32_t length = ftell(f) + 1;
	fseek(f, 0, SEEK_SET);
	*dest = calloc(length, sizeof(char));
	ASSERT(*dest);

	fread(*dest, sizeof(char), length, f);
	fclose(f);
}

static void load_score(void)
{
	FILE *f = fopen(FILE_SCORE, "r");

	if (!f)
	{
		return;
	}

	fscanf(f, "%d;", &g_score.record);
	fclose(f);
}
