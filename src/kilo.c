
// NEXT: https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html

#include "kilo.h"

struct s_editor_config	g_editor;

void	init_editor(void)
{
	g_editor.cursor.x = 0;
	g_editor.cursor.y = 0;
	g_editor.num_rows = 0;

	if (get_window_size(&g_editor.screen_rows, &g_editor.screen_cols) == ERROR)
		exit_error("get_window_size");
}

/* File i/o */

void	editor_open(void)
{
	char	*line = "Hello, world!";
	ssize_t	linelen = 13;

	g_editor.row.size = linelen;
	g_editor.row.chars = malloc(linelen + 1);
	memcpy(g_editor.row.chars, line, linelen);
	g_editor.row.chars[linelen] = '\0';
	g_editor.num_rows = 1;
}


// 

int	main(void)
{
	enable_raw_mode();
	init_editor();
	editor_open();
	
	while (true)
	{
		editor_refresh_screen();
		editor_process_keypress();
	}

	return (0);
}
