
#include "utils.h"

void	editor_refresh_screen(void)
{
	write(STDOUT, CLEAR_SCREEN, 4);
	write(STDOUT, CURSOR_TOP_LEFT, 3);
}

void	exit_error(const char *str)
{
	editor_refresh_screen();
	perror(str);
	exit(FAILURE);
}
