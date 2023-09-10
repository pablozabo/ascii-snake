#include "common.h"

#define LOG_FILE "log.txt"

void error_handler(const char *file, const char *function, int line, const char *exp)
{
	char message[1024] = { '\0' };
	sprintf(message, "error on %s, function %s, line %d, expression: %s\n", file, function, line, exp);
	printf("\n# ERROR! # => %s\n", message);

	FILE *log_file = fopen(LOG_FILE, "a+");

	if (log_file)
	{
		fprintf(log_file, "%s", message);
		fclose(log_file);
	}

	exit(1);
}

void set_offset_yx(uint8_t height, uint8_t width, uint8_t *offset_y, uint8_t *offset_x)
{
	uint8_t rows, cols;
	getmaxyx(stdscr, rows, cols);

	*offset_y = (rows - height) * 0.5;
	*offset_x = (cols - width) * 0.5;
}
