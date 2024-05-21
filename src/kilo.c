
// NEXT: 

#include "kilo.h"

struct s_editor_config	g_editor;

#include <sys/ioctl.h>

int	get_window_size(int *rows, int *cols)
{
	struct winsize	ws;

	if (ioctl(STDOUT, TIOCGWINSZ, &ws) == ERROR || ws.ws_col == 0)
	{
		return (ERROR);
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return (0);
	}
}

void	init_editor(void)
{
	if (get_window_size(&g_editor.screen_rows, &g_editor.screen_cols) == ERROR)
		exit_error("get_window_size");
}

int	main(void)
{
	enable_raw_mode();
	init_editor();
	while (true)
	{
		editor_refresh_screen();
		editor_process_keypress();
	}

	return (0);
}
