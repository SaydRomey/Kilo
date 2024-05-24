
// NEXT: https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html

#include "kilo.h"

struct s_editor_config	g_editor;

#include <sys/ioctl.h>

int	get_cursor_pos(int *rows, int *cols)
{
	char			buffer[32];
	unsigned int	i;

	if (ft_putstr("\x1b[6n") != 4)
		return (ERROR);
	i = 0;
	while (i < sizeof(buffer) - 1)
	{
		if (read(STDIN, &buffer[i], 1) != 1)
			break ;
		if (buffer[i] == 'R')
			break ;
		i++;
	}
	buffer[i] = '\0';

	if (buffer[0] != '\x1b' || buffer[1] != '[')
		return (ERROR);
	if (sscanf(&buffer[2], "%d;%d", rows, cols) != 2)
		return (ERROR);
	
	return (0);
}

int	get_window_size(int *rows, int *cols)
{
	struct winsize	ws;

	if (ioctl(STDOUT, TIOCGWINSZ, &ws) == ERROR || ws.ws_col == 0)
	{
		// if (ft_putstr("\x1b[999c\x1b[999b") != 12)
			// return (ERROR);
		// return (get_cursor_pos(rows, cols));
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
