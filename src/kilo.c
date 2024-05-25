
// NEXT: https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html

#include "kilo.h"

struct s_editor_config	g_editor;


/* init */

void	init_editor(void)
{
	g_editor.cursor.x = 0;
	g_editor.cursor.y = 0;
	g_editor.row_offset = 0;
	g_editor.num_rows = 0;
	g_editor.row = NULL;

	if (get_window_size(&g_editor.screen_rows, &g_editor.screen_cols) == ERROR)
		exit_error("get_window_size");
}

// 

/* row operations */

void	editor_append_row(char *str, size_t len)
{
	g_editor.row = realloc(g_editor.row, sizeof(t_editor_row) * (g_editor.num_rows + 1));
	
	int	at = g_editor.num_rows;
	g_editor.row[at].size = len;
	g_editor.row[at].chars = malloc(len + 1);
	memcpy(g_editor.row[at].chars, str, len);
	g_editor.row[at].chars[len] = '\0';
	g_editor.num_rows++;
}

/* file i/o */

void	editor_open(char *filename)
{
	FILE	*fp = fopen(filename, "r");
	if (!fp)
		exit_error("fopen");
	
	char	*line = NULL;
	size_t	linecap = 0;
	ssize_t	linelen;

	while ((linelen = getline(&line, &linecap, fp)) != -1)
	{
		while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
		{
			linelen--;
		}
		editor_append_row(line, linelen);
	}
	free(line);
	fclose(fp);
}


// 

int	main(int argc, char *argv[])
{
	enable_raw_mode();
	init_editor();
	if (argc >= 2)
	{
		editor_open(argv[1]);
	}
	while (true)
	{
		editor_refresh_screen();
		editor_process_keypress();
	}

	return (0);
}
