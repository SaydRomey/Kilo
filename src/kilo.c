
#include "kilo.h"

struct s_editor_config	E;


/* init */

void	init_editor(void)
{
	E.cursor.x = 0;
	E.cursor.y = 0;
	E.render_x = 0;
	E.row_offset = 0;
	E.col_offset = 0;
	E.num_rows = 0;
	E.row = NULL;
	E.filename = NULL;

	if (get_window_size(&E.screen_rows, &E.screen_cols) == ERROR)
		exit_error("get_window_size");
	E.screen_rows -= 1;
}

// 

/* row operations */

int	editor_row_cursorx_to_render_x(t_editor_row *row, int cursor_x)
{
	int	render_x;
	int	j;

	render_x = 0;
	j = 0;
	while (j < cursor_x)
	{
		if (row->chars[j] == '\t')
			render_x += (KILO_TAB_STOP - 1) - (render_x % KILO_TAB_STOP);
		render_x++;
		j++;
	}
	return (render_x);
}

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
	E.row = realloc(E.row, sizeof(t_editor_row) * (E.num_rows + 1));
	
	int	at = E.num_rows;
	E.row[at].size = len;
	E.row[at].chars = malloc(len + 1);
	memcpy(E.row[at].chars, str, len);
	E.row[at].chars[len] = '\0';
	
	E.row[at].rsize = 0;
	E.row[at].render = NULL;
	editor_update_row(&E.row[at]);

	E.num_rows++;
}

/* file i/o */

void	editor_open(char *filename)
{
	free(E.filename);
	E.filename = strdup(filename);

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
