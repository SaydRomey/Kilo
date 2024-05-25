
#include "kilo.h"

static void	welcome_msg(struct s_abuf *ab)
{
	char	welcome[80];
	int		welcome_len;
	int		padding;

	welcome_len = snprintf(welcome, sizeof(welcome), \
		"Kilo editor -- version %s", KILO_VERSION);
	if (welcome_len > g_editor.screen_cols)
		welcome_len = g_editor.screen_cols;
	padding = (g_editor.screen_cols - welcome_len) / 2;
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

	y = 0;
	while (y < g_editor.screen_rows)
	{
		if (y >= g_editor.num_rows)
		{
			if (g_editor.num_rows == 0 && y == g_editor.screen_rows / 3)
				welcome_msg(ab);
			else
			{
				ab_append(ab, "~", 1);
			}
		}
		else
		{
			int	len = g_editor.row[y].size;
			if (len > g_editor.screen_cols)
				len = g_editor.screen_cols;
			ab_append(ab, g_editor.row[y].chars, len);
		}
		ab_append(ab, "\x1b[K", 3);
		if (y < g_editor.screen_rows - 1)
			ab_append(ab, "\r\n", 2);
		y++;
	}
}

static void	set_cursor(struct s_abuf *ab)
{
	char	buf[32];
	t_point	*cursor;

	cursor = &g_editor.cursor;
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cursor->y + 1, cursor->x + 1);
	ab_append(ab, buf, ft_strlen(buf));
}

void	editor_refresh_screen(void)
{
	struct s_abuf	ab = ABUF_INIT;

	ab_append(&ab, "\x1b[?25l", 6);
	ab_append(&ab, "\x1b[H", 3);

	editor_draw_rows(&ab);

	set_cursor(&ab);
	ab_append(&ab, "\x1b[?25l", 6);

	ft_putstr(ab.buffer);
	ab_free(&ab);
}
