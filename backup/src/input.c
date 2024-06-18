
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
						case '3':
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
			switch (seq[1])
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
	t_editor_row	*row;

	if (E.cursor.y >= E.num_rows)
		row = NULL;
	else
		row = &E.row[E.cursor.y];
	switch (key)
	{
		case ARROW_UP:
			if (E.cursor.y != 0)
			{
				E.cursor.y--;
			}
			break;
		case ARROW_LEFT:
			if (E.cursor.x != 0)
			{
				E.cursor.x--;
			}
			else if (E.cursor.y > 0)
			{
				E.cursor.y--;
				E.cursor.x = E.row[E.cursor.y].size;
			}
			break;
		case ARROW_DOWN:
			if (E.cursor.y < E.num_rows)
			{
				E.cursor.y++;
			}
			break;
		case ARROW_RIGHT:
			if (row && E.cursor.x < row->size)
			{
				E.cursor.x++;
			}
			else if (row && E.cursor.x == row->size)
			{
				E.cursor.y++;
				E.cursor.x = 0;
			}
			break;
	}

	if (E.cursor.y >= E.num_rows)
		row = NULL;
	else
		row = &E.row[E.cursor.y];
	int	row_len;
	if (row)
		row_len = row->size;
	else
		row_len = 0;
	if (E.cursor.x > row_len)
	{
		E.cursor.x = row_len;
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
			E.cursor.x = 0;
			break ;
		
		case END_KEY:
			if (E.cursor.y < E.num_rows)
				E.cursor.x = E.row[E.cursor.y].size;
			break ;
		
		case PAGE_UP:
		case PAGE_DOWN:
			{
				if (c == PAGE_UP)
				{
					E.cursor.y = E.row_offset;
				}
				else if (c == PAGE_DOWN)
				{
					E.cursor.y = E.row_offset + E.screen_rows - 1;
					if (E.cursor.y > E.num_rows)
						E.cursor.y = E.num_rows;
				}
				times = E.screen_rows;
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
