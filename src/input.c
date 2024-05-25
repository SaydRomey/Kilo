
#include "kilo.h"

/* terminal */

int	editor_read_key(void)
{
	int		nread;
	char	c;
	char	seq[3];

	while ((nread = read(STDIN, &c, 1)) != 1)
	{
		if (nread == ERROR && errno != EAGAIN)
			exit_error("read");
	}
	if (c == '\x1b')
	{
		if (read(STDIN, &seq[0], 1) != 1)
			return ('\x1b');
		if (read(STDIN, &seq[1], 1) != 1)
			return ('\x1b');
		
		if (seq[0] == '[')
		{
			if (seq[1] >= '0' && seq[1] <= '9')
			{
				if (read(STDIN, &seq[2], 1) != 1)
					return ('\x1b');
				if (seq[2] == '~')
				{
					switch (seq[1])
					{
						case '1':
							return (HOME_KEY);
						case '2':
							return (DEL_KEY);
						case '4':
							return (END_KEY);
						case '5':
							return (PAGE_UP);
						case '6':
							return (PAGE_DOWN);
						case '7':
							return (HOME_KEY);
						case '8':
							return (END_KEY);
					}
				}
			}
			else
			{
				switch (seq[1])
				{
					case 'A':
						return (ARROW_UP);
					case 'B':
						return (ARROW_DOWN);
					case 'C':
						return (ARROW_RIGHT);
					case 'D':
						return (ARROW_LEFT);
					case 'H':
						return (HOME_KEY);
					case 'F':
						return (END_KEY);
				}
			}
		}
		else if (seq[0] == 'O')
		{
			switch (seq[0])
			{
				case 'H':
					return (HOME_KEY);
				case 'F':
					return (END_KEY);
			}
		}
		return ('\x1b');
	}
	else
		return (c);
}

/* input */

void	editor_move_cursor(int key)
{
	switch (key)
	{
		case ARROW_UP:
			if (g_editor.cursor.y != 0)
			{
				g_editor.cursor.y--;
			}
			break;
		case ARROW_LEFT:
			if (g_editor.cursor.x != 0)
			{
				g_editor.cursor.x--;
			}
			break;
		case ARROW_DOWN:
			if (g_editor.cursor.y != g_editor.screen_rows - 1)
			{
				g_editor.cursor.y++;
			}
			break;
		case ARROW_RIGHT:
			if (g_editor.cursor.x != g_editor.screen_cols - 1)
			{
				g_editor.cursor.x++;
			}
			break;
	}
}

void	editor_process_keypress(void)
{
	int	c;
	int	times;

	c = editor_read_key();
	switch (c)
	{
		case CTRL_KEY('q'):
			ft_putstr(CLEAR_SCREEN);
			ft_putstr(CURSOR_TOP_LEFT);
			exit(0);
			break;
		
		case HOME_KEY:
			g_editor.cursor.x = 0;
			break ;
		
		case END_KEY:
			g_editor.cursor.x = g_editor.screen_cols - 1;
			break ;
		
		case PAGE_UP:
		case PAGE_DOWN:
			{
				times = g_editor.screen_rows;
				while (times--)
				{
					if (c == PAGE_UP)
						editor_move_cursor(ARROW_UP);
					else
						editor_move_cursor(ARROW_DOWN);
				}
			}
			break;
		
		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			editor_move_cursor(c);
			break ;
	}
}
