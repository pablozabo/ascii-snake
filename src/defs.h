#ifndef DEFS_H
#define DEFS_H

#include <curses.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// TYPES
typedef signed char	   int8_t;
typedef unsigned char  uint8_t;
typedef signed short   int16_t;
typedef unsigned short uint16_t;
typedef int			   int32_t;
typedef unsigned	   uint32_t;
typedef float		   float32_t;
typedef void (*screen_action_t)(void);
typedef bool (*screen_is_completed_t)(void);

// DEFS
#define CURRENT_TIME (1000.0 * clock() / CLOCKS_PER_SEC)

#define CH_SHAPE_FILL ACS_CKBOARD
#define CH_EOL '\n'
#define CH_EOS '\0'
#define CH_ENTER 10
#define CH_ESC 27

#define FILE_SCORE "score.txt"

typedef enum color_pair_t
{
	COLOR_PAIR_BLUE	   = 1,
	COLOR_PAIR_BLUE_BK = 2,
	COLOR_PAIR_RED	   = 3,
	COLOR_PAIR_RED_BK  = 4,
	COLOR_PAIR_GREEN   = 5
} color_pair_t;

typedef struct vec2_t
{
	int8_t x;
	int8_t y;
} vec2_t;

typedef struct score_t
{
	uint32_t current;
	uint32_t record;
} score_t;

#endif