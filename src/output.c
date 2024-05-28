
#include "kilo.h"

void	editor_scroll(void)
{
	E.render_x = 0;

	if (E.cursor.x < E.num_rows)
	{
		E.render_x = editor_row_cursorx_to_render_x(&E.row[E.cursor.y], E.cursor.x);
	}

	if (E.cursor.y < E.row_offset)
	{
		E.row_offset = E.cursor.y;
	}
	if (E.cursor.y >= E.row_offset + E.screen_rows)
	{
		E.row_offset = E.cursor.y - E.screen_rows + 1;
	}
	if (E.render_x < E.col_offset)
	{
		E.col_offset = E.render_x;
	}
	if (E.render_x >= E.col_offset + E.screen_cols)
	{
		E.col_offset = E.render_x - E.screen_cols + 1;
	}
}

static void	welcome_msg(struct s_abuf *ab)
{
	char	welcome[80];
	int		welcome_len;
	int		padding;

	welcome_len = snprintf(welcome, sizeof(welcome), \
		"Kilo editor -- version %s", KILO_VERSION);
	if (welcome_len > E.screen_cols)
		welcome_len = E.screen_cols;
	padding = (E.screen_cols - welcome_len) / 2;
	if (padding)
	{
		ab_append(ab, "~", 1);
		padding--;
	}
	while (padding)
	{
		ab_append(ab, " ", 1);
		padding--;
	}
	ab_append(ab, welcome, welcome_len);
}

void	editor_draw_rows(struct s_abuf *ab)
{
	int	y;
	int	file_row;

	y = 0;
	while (y < E.screen_rows)
	{
		file_row = y + E.row_offset;
		if (file_row >= E.num_rows)
		{
			if (E.num_rows == 0 && y == E.screen_rows / 3)
				welcome_msg(ab);
			else
			{
				ab_append(ab, "~", 1);
			}
		}
		else
		{
			int	len = E.row[file_row].rsize - E.col_offset;
			if (len < 0)
				len = 0;
			if (len > E.screen_cols)
				len = E.screen_cols;
			ab_append(ab, &E.row[file_row].render[E.col_offset], len);
		}
		ab_append(ab, "\x1b[K", 3);
		ab_append(ab, "\r\n", 2);
		y++;
	}
}

void	editor_draw_status_bar(struct s_abuf *ab)
{
	char	status[80];
	char	rstatus[80];
	char	*filename;
	int		len;
	int		rlen;

	ab_append(ab, "\x1b[7m", 4);
	if (E.filename)
		filename = E.filename;
	else
		filename = "[No Name]";
	len = snprintf(status, sizeof(status), "%.20s - %d lines", filename, E.num_rows);
	rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cursor.y + 1, E.num_rows);
	if (len > E.screen_cols)
		len = E.screen_cols;
	ab_append(ab, status, len);
	while (len < E.screen_cols)
	{
		if (E.screen_cols - len == rlen)
		{
			ab_append(ab, rstatus, rlen);
			break ;
		}
		else
		{
			ab_append(ab, " ", 1);
			len++;
		}
	}
	ab_append(ab, "\x1b[m", 3);
}

static void	set_cursor(struct s_abuf *ab)
{
	char	buf[32];

	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", \
	(E.cursor.y - E.row_offset) + 1, \
	(E.render_x - E.col_offset) + 1);
	ab_append(ab, buf, ft_strlen(buf));
}

void	editor_refresh_screen(void)
{
	editor_scroll();

	struct s_abuf	ab = ABUF_INIT;

	ab_append(&ab, "\x1b[?25l", 6);
	ab_append(&ab, "\x1b[H", 3);

	editor_draw_rows(&ab);
	editor_draw_status_bar(&ab);

	set_cursor(&ab);
	ab_append(&ab, "\x1b[?25l", 6);

	ft_putstr(ab.buffer);
	ab_free(&ab);
}
