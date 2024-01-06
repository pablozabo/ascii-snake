#include "screen_game.h"
#include "../common.h"
#include "../data_structures/data_structures.h"

extern int		 g_key;
extern score_t	 g_score;
extern float32_t g_delta_time;

#define COORDS_TO_INDEX(x, y) (win_board_height * y + x)
#define SET_BOARD_CELL_VAL(x, y, val) (*(board_model + COORDS_TO_INDEX(x, y)) = val, val ? board_cell_pool_remove(x, y) : board_cell_pool_add(x, y))
#define GET_BOARD_CELL_VAL(x, y) (*(board_model + COORDS_TO_INDEX(x, y)))

#define CH_SNAKE_TONGE_LEFT ACS_LLCORNER
#define CH_SNAKE_TONGE_RIGHT ACS_URCORNER
#define CH_SNAKE_TONGE_TOP ACS_ULCORNER
#define CH_SNAKE_TONGE_BOTTOM ACS_LRCORNER

typedef struct snake_node_t
{
	vec2_t				 curr_pos;
	vec2_t				 prev_pos;
	struct snake_node_t *next_node; // <==
	struct snake_node_t *prev_node; // ==>
} snake_node_t;

typedef enum snake_direction_t
{
	SNAKE_DIRECTION_IDLE   = 0,
	SNAKE_DIRECTION_LEFT   = 1,
	SNAKE_DIRECTION_RIGHT  = 2,
	SNAKE_DIRECTION_TOP	   = 3,
	SNAKE_DIRECTION_BOTTOM = 4
} snake_direction_t;

typedef struct snake_t
{
	snake_node_t	 *first_node;
	snake_node_t	 *head;
	snake_node_t	 *tail;
	chtype			  tonge_ch;
	float32_t		  elapsed_time;
	float32_t		  collided_elapsed_time;
	float32_t		  speed;
	float32_t		  max_speed;
	float32_t		  acceleration;
	uint8_t			  length; // number of active nodes
	snake_direction_t direction;
	bool			  collided;
} snake_t;

typedef enum fruit_status_t
{
	FRUIT_STATUS_IDLE	= 0,
	FRUIT_STATUS_ACTIVE = 1,
	FRUIT_STATUS_EATEN	= 2
} fruit_status_t;

typedef struct fruit_t
{
	vec2_t		   pos;
	float32_t	   lifetime;
	float32_t	   elapsed_time;
	fruit_status_t status;
} fruit_t;

typedef struct fruit_pool_t
{
	fruit_t	 *fruits;
	float32_t elapsed_time;
	float32_t rand_time_to_activate_fruit;
	uint8_t	  length; // number of fruits
} fruit_pool_t;

// As fruits are placed randomly on board,
// we must check if the generated random cell (x,y)
// is available on 'board_model' and keep generating random values
// until we find a free 'board_model' cell.
// As the snake starts growing, this solution gets
// slower and slower as more iterations are need to
// find a free cell.
// A better alternative is to store all available cells in a pool,
// so we only need to generate a random index to retrieve a cell.
// Sparse set offers fast index search/insert/delete operations and
// a companion 'cells' vector stores its values.
// for more information about sparse sets: https://www.geeksforgeeks.org/sparse-set/
typedef struct board_cell_pool_t
{
	sparse_set_t indexes;
	vec2_t		*cells;
} board_cell_pool_t;

static const uint8_t win_board_height  = 40;
static const uint8_t win_board_width   = win_board_height * 2;
static const uint8_t win_board_padding = 3;
static const uint8_t win_score_width   = 80;
static const uint8_t win_score_height  = 1;

static const float32_t snake_speed_init			= 0.5;
static const float32_t snake_speed_max			= 0.1;
static const float32_t snake_speed_acceleration = 0.01;

static const float32_t fruit_lifetime	  = 15;
static const uint32_t  points_movement	  = 10;
static const uint32_t  points_fruit_eaten = 50;

static WINDOW *win_board;
static WINDOW *win_score;

static snake_t			 snake;
static fruit_pool_t		 fruit_pool;
static board_cell_pool_t board_cell_pool;
// board_model is required to keep track which cells are filled
// with snake body nodes and detect collisions quickly
static bool *board_model = NULL;

static void handle_input(void);
static void move_snake(void);
static void update_fruit_pool(void);
static void check_eaten_fruits(void);
static void check_collision(void);
static void board_cell_pool_add(uint8_t x, uint8_t y);
static void board_cell_pool_remove(uint8_t x, uint8_t y);
static void save_score(void);
static void render_board(void);
static void render_snake(void);
static void render_fruits(void);
static void render_score(void);

void screen_game_init(void)
{
	uint8_t offset_y, offset_x;
	srand(time(NULL));
	g_score.current = 0;
	// win init
	set_offset_yx(win_board_height, win_board_width, &offset_y, &offset_x);
	win_board = newwin(win_board_height, win_board_width, offset_y, offset_x);
	scrollok(win_board, TRUE);
	win_score = newwin(win_score_height, win_score_width, offset_y - 1, offset_x);
	scrollok(win_score, TRUE);

	// board model init
	board_model = calloc(sizeof(bool), win_board_height * win_board_height);
	ASSERT(board_model);

	// snake init
	snake.first_node = snake.head = snake.tail = calloc(sizeof(snake_node_t), win_board_height * win_board_height);
	ASSERT(snake.first_node);

	snake.direction				= SNAKE_DIRECTION_LEFT;
	snake.tonge_ch				= CH_SNAKE_TONGE_LEFT;
	snake.speed					= snake_speed_init;
	snake.max_speed				= snake_speed_max;
	snake.acceleration			= snake_speed_acceleration;
	snake.length				= 1;
	snake.collided				= false;
	snake.collided_elapsed_time = 0;
	snake.elapsed_time			= 0;

	// board cell pool init
	board_cell_pool.indexes = sparse_set_new(win_board_height * win_board_height);
	board_cell_pool.cells	= malloc(sizeof(vec2_t) * win_board_height * win_board_height);
	ASSERT(board_cell_pool.cells);

	// fill available cells, avoiding board edges
	for (uint8_t y = win_board_padding; y < win_board_height - win_board_padding; y++)
	{
		for (uint8_t x = win_board_padding; x < win_board_height - win_board_padding; x++)
		{
			uint16_t index = COORDS_TO_INDEX(x, y);
			sparse_set_add(&board_cell_pool.indexes, index);
			vec2_t *cell = (board_cell_pool.cells + index);
			(*cell).x	 = x;
			(*cell).y	 = y;
		}
	}

	// fruit pool init
	fruit_pool.length = 6;
	fruit_pool.fruits = calloc(sizeof(fruit_t), fruit_pool.length);
	ASSERT(fruit_pool.fruits);

	snake.head->curr_pos.x = win_board_height * 0.5;
	snake.head->curr_pos.y = win_board_height * 0.5;
	SET_BOARD_CELL_VAL(snake.head->curr_pos.x, snake.head->curr_pos.y, true);

	render_score();
	render_board();
}

void screen_game_dispose(void)
{
	save_score();
	wclear(win_board);
	wrefresh(win_board);
	delwin(win_board);

	wclear(win_score);
	wrefresh(win_score);
	delwin(win_score);

	free(board_model);
	free(snake.first_node);
	free(board_cell_pool.cells);
	sparse_set_dispose(&board_cell_pool.indexes);
}

bool screen_game_is_completed(void)
{
	return snake.collided && snake.collided_elapsed_time > 4;
}

void screen_game_update(void)
{
	if (snake.collided)
	{
		snake.collided_elapsed_time += g_delta_time;
		return;
	}

	handle_input();
	update_fruit_pool();
	snake.elapsed_time += g_delta_time;

	if (snake.elapsed_time >= snake.speed)
	{
		move_snake();
		check_eaten_fruits();
		check_collision();

		SET_BOARD_CELL_VAL(snake.head->curr_pos.x, snake.head->curr_pos.y, true);
		g_score.current += points_movement;
		snake.elapsed_time = 0;
	}
}

void screen_game_render(void)
{
	render_score();

	wclear(win_board);
	render_board();
	render_fruits();
	render_snake();
	wrefresh(win_board);
}

static void handle_input(void)
{
	if (g_key > 0)
	{
		if (g_key == KEY_UP)
		{
			snake.direction = SNAKE_DIRECTION_TOP;
			snake.tonge_ch	= CH_SNAKE_TONGE_TOP;
		}
		else if (g_key == KEY_DOWN)
		{
			snake.direction = SNAKE_DIRECTION_BOTTOM;
			snake.tonge_ch	= CH_SNAKE_TONGE_BOTTOM;
		}
		else if (g_key == KEY_LEFT)
		{
			snake.direction = SNAKE_DIRECTION_LEFT;
			snake.tonge_ch	= CH_SNAKE_TONGE_LEFT;
		}
		else if (g_key == KEY_RIGHT)
		{
			snake.direction = SNAKE_DIRECTION_RIGHT;
			snake.tonge_ch	= CH_SNAKE_TONGE_RIGHT;
		}
	}
}

static void move_snake()
{
	SET_BOARD_CELL_VAL(snake.tail->curr_pos.x, snake.tail->curr_pos.y, false);
	snake.tail->prev_pos.x = snake.head->curr_pos.x;
	snake.tail->prev_pos.y = snake.head->curr_pos.y;

	// just move the tail to the front (head)
	if (snake.length > 1)
	{
		snake_node_t *new_tail = snake.tail->prev_node;
		snake.tail->curr_pos.x = snake.head->curr_pos.x;
		snake.tail->curr_pos.y = snake.head->curr_pos.y;

		snake.head->prev_node			 = snake.tail;
		snake.tail->next_node			 = snake.head;
		snake.tail->prev_node->next_node = NULL;
		snake.tail->prev_node			 = NULL;
		snake.head						 = snake.tail;
		snake.tail						 = new_tail;
	}

	if (snake.direction == SNAKE_DIRECTION_TOP)
	{
		snake.head->curr_pos.y -= 1;
	}
	else if (snake.direction == SNAKE_DIRECTION_BOTTOM)
	{
		snake.head->curr_pos.y += 1;
	}
	else if (snake.direction == SNAKE_DIRECTION_LEFT)
	{
		snake.head->curr_pos.x -= 1;
	}
	else if (snake.direction == SNAKE_DIRECTION_RIGHT)
	{
		snake.head->curr_pos.x += 1;
	}
}

static void update_fruit_pool(void)
{
	fruit_pool.elapsed_time += g_delta_time;

	for (uint8_t i = 0; i < fruit_pool.length; i++)
	{
		fruit_t *fruit = &fruit_pool.fruits[i];

		if (fruit->status == FRUIT_STATUS_ACTIVE)
		{
			fruit->elapsed_time += g_delta_time;

			if (fruit->elapsed_time > fruit->lifetime)
			{
				fruit->status = FRUIT_STATUS_IDLE;
				board_cell_pool_add(fruit->pos.x, fruit->pos.y);
			}
		}
		else if (fruit->status == FRUIT_STATUS_IDLE && fruit_pool.elapsed_time > fruit_pool.rand_time_to_activate_fruit)
		{
			fruit->status			= FRUIT_STATUS_ACTIVE;
			fruit->elapsed_time		= 0;
			fruit->lifetime			= fruit_lifetime;
			fruit_pool.elapsed_time = 0;

			fruit_pool.rand_time_to_activate_fruit = 3 + rand() % 3; // between 3 and 6 seconds;

			// add the fruit in a free board cell
			uint16_t available_cells_length = VECTOR_LENGTH(board_cell_pool.indexes.dense);

			if (available_cells_length)
			{
				uint16_t index = rand() % available_cells_length;
				vec2_t	*cell  = (board_cell_pool.cells + index);

				fruit->pos.x = cell->x;
				fruit->pos.y = cell->y;

				board_cell_pool_remove(cell->x, cell->y);
			}
		}
	}
}

static void check_eaten_fruits(void)
{
	uint8_t head_x = snake.head->curr_pos.x * 2;
	uint8_t head_y = snake.head->curr_pos.y;
	uint8_t tail_x = snake.tail->prev_pos.x * 2;
	uint8_t tail_y = snake.tail->prev_pos.y;

	for (uint8_t i = 0; i < fruit_pool.length; i++)
	{
		fruit_t *fruit = &fruit_pool.fruits[i];

		if (fruit->status == FRUIT_STATUS_ACTIVE &&
			(head_x == fruit->pos.x || (head_x + 1) == fruit->pos.x) &&
			head_y == fruit->pos.y)
		{
			fruit->status = FRUIT_STATUS_EATEN;
			g_score.current += points_fruit_eaten;

			if (snake.speed > snake.max_speed)
			{
				snake.speed -= snake.acceleration;
			}
		}
		else if (fruit->status == FRUIT_STATUS_EATEN &&
				 (tail_x == fruit->pos.x || (tail_x + 1) == fruit->pos.x) &&
				 tail_y == fruit->pos.y)
		{
			snake_node_t *next_node = (snake.first_node + snake.length);
			next_node->curr_pos.x	= snake.tail->prev_pos.x;
			next_node->curr_pos.y	= snake.tail->prev_pos.y;

			snake.tail->next_node = next_node;
			next_node->prev_node  = snake.tail;
			snake.tail			  = next_node;
			snake.length++;

			fruit->status = FRUIT_STATUS_IDLE;
		}
	}
}

static void check_collision()
{
	uint8_t head_x = snake.head->curr_pos.x;
	uint8_t head_y = snake.head->curr_pos.y;

	snake.collided = head_x <= 0 ||
					 head_x >= (win_board_height - 1) ||
					 head_y <= 0 ||
					 head_y >= (win_board_height - 1) ||
					 GET_BOARD_CELL_VAL(head_x, head_y);
}

static void board_cell_pool_add(uint8_t x, uint8_t y)
{
	if (x <= win_board_padding ||
		y <= win_board_padding ||
		x >= (win_board_height - win_board_padding) ||
		y >= (win_board_height - win_board_padding))
	{
		return;
	}

	uint16_t index = COORDS_TO_INDEX(x, y);
	sparse_set_add(&board_cell_pool.indexes, index);
	uint16_t available_cells_length = VECTOR_LENGTH(board_cell_pool.indexes.dense);
	vec2_t	*cell					= (board_cell_pool.cells + available_cells_length - 1);

	cell->x = x;
	cell->y = y;
}

static void board_cell_pool_remove(uint8_t x, uint8_t y)
{
	if (x <= win_board_padding ||
		y <= win_board_padding ||
		x >= (win_board_height - win_board_padding) ||
		y >= (win_board_height - win_board_padding))
	{
		return;
	}

	int32_t coord_index = COORDS_TO_INDEX(x, y);
	int16_t index		= SPARSE_SET_INDEXOF(board_cell_pool.indexes, coord_index);

	if (index < 0)
	{
		return;
	}

	uint16_t available_cells_length = VECTOR_LENGTH(board_cell_pool.indexes.dense);
	vec2_t	*cell_origin			= (board_cell_pool.cells + available_cells_length - 1);
	vec2_t	*cell_dest				= (board_cell_pool.cells + index);

	cell_dest->x = cell_origin->x;
	cell_dest->y = cell_origin->y;
	sparse_set_remove(&board_cell_pool.indexes, coord_index);
}

static void save_score(void)
{
	if (g_score.current <= g_score.record)
	{
		return;
	}

	g_score.record = g_score.current;

	FILE *f = fopen(FILE_SCORE, "w");
	ASSERT(f);

	fprintf(f, "%d", g_score.record);
	fclose(f);
}

static void render_board(void)
{
	wattron(win_board, COLOR_PAIR(COLOR_PAIR_GREEN));
	box(win_board, 0, 0);
	wattroff(win_board, COLOR_PAIR(COLOR_PAIR_GREEN));
}

static void render_snake(void)
{
	snake_node_t *node_aux = snake.head;

	wattron(win_board, COLOR_PAIR(COLOR_PAIR_RED));

	// tonge
	if (snake.direction == SNAKE_DIRECTION_TOP)
	{
		mvwaddch(win_board, node_aux->curr_pos.y - 1, node_aux->curr_pos.x * 2 + 1, snake.tonge_ch);
	}
	else if (snake.direction == SNAKE_DIRECTION_BOTTOM)
	{
		mvwaddch(win_board, node_aux->curr_pos.y + 1, node_aux->curr_pos.x * 2, snake.tonge_ch);
	}
	else if (snake.direction == SNAKE_DIRECTION_LEFT || snake.direction == SNAKE_DIRECTION_IDLE)
	{
		mvwaddch(win_board, node_aux->curr_pos.y, node_aux->curr_pos.x * 2 - 1, snake.tonge_ch);
	}
	else if (snake.direction == SNAKE_DIRECTION_RIGHT)
	{
		mvwaddch(win_board, node_aux->curr_pos.y, node_aux->curr_pos.x * 2 + 2, snake.tonge_ch);
	}

	wattroff(win_board, COLOR_PAIR(COLOR_PAIR_RED));

	uint8_t snake_color = snake.collided && (uint32_t)(snake.collided_elapsed_time * 5) % 2 ? COLOR_PAIR_RED : COLOR_PAIR_GREEN;
	wattron(win_board, COLOR_PAIR(snake_color));

	// body
	while (node_aux)
	{
		mvwaddch(win_board, node_aux->curr_pos.y, node_aux->curr_pos.x * 2, CH_SHAPE_FILL);
		mvwaddch(win_board, node_aux->curr_pos.y, (node_aux->curr_pos.x * 2) + 1, CH_SHAPE_FILL);
		node_aux = node_aux->next_node;
	}

	wattroff(win_board, COLOR_PAIR(snake_color));
}

static void render_fruits(void)
{
	for (uint8_t i = 0; i < fruit_pool.length; i++)
	{
		fruit_t fruit = fruit_pool.fruits[i];

		if (fruit.status == FRUIT_STATUS_ACTIVE)
		{
			mvwaddch(win_board, fruit.pos.y, fruit.pos.x, ACS_DIAMOND);
		}
	}
}

static void render_score(void)
{
	char max_score[20]	   = { '\0' };
	char current_score[20] = { '\0' };

	sprintf(max_score, "Max score: %d", g_score.record);
	sprintf(current_score, "Current score: %d", g_score.current);
	uint8_t x = win_score_width - strlen(current_score);

	wclear(win_score);
	mvwprintw(win_score, 0, 1, "%s", max_score);
	mvwprintw(win_score, 0, x - 1, "%s", current_score);

	wrefresh(win_score);
}