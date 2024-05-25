
// NEXT: https://viewsourcecode.org/snaptoken/kilo/04.aTextViewer.html
	// (Tabs and the cursor)

#include "kilo.h"

struct s_editor_config	g_editor;


/* init */

void	init_editor(void)
{
	g_editor.cursor.x = 0;
	g_editor.cursor.y = 0;
	g_editor.row_offset = 0;
	g_editor.col_offset = 0;
	g_editor.num_rows = 0;
	g_editor.row = NULL;

	if (get_window_size(&g_editor.screen_rows, &g_editor.screen_cols) == ERROR)
		exit_error("get_window_size");
}

// 

/* row operations */

void	editor_update_row(t_editor_row *row)
{
	int	idx;
	int	j;
	int	tabs;

	j = 0;
	tabs = 0;
	while (j < row->size)
	{
		if (row->chars[j] == '\t')
			tabs++;
		j++;
	}

	free(row->render);
	row->render = malloc(row->size + tabs*(KILO_TAB_STOP - 1) + 1);
	idx = 0;
	j = 0;
	while (j < row->size)
	{
		if (row->chars[j] == '\t')
		{
			row->render[idx++] = ' ';
			while (idx % KILO_TAB_STOP != 0)
			{
				row->render[idx++] = ' ';
			}
		}
		else
		{
			row->render[idx++] = row->chars[j];
		}
		j++;
	}
	row->render[idx] = '\0';
	row->rsize = idx;
}

void	editor_append_row(char *str, size_t len)
{
	g_editor.row = realloc(g_editor.row, sizeof(t_editor_row) * (g_editor.num_rows + 1));
	
	int	at = g_editor.num_rows;
	g_editor.row[at].size = len;
	g_editor.row[at].chars = malloc(len + 1);
	memcpy(g_editor.row[at].chars, str, len);
	g_editor.row[at].chars[len] = '\0';
	
	g_editor.row[at].rsize = 0;
	g_editor.row[at].render = NULL;
	editor_update_row(&g_editor.row[at]);

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
