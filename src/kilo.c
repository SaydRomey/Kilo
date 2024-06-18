
#include "kilo.h"

/******************************************************************************/// data

typedef struct s_editor_row
{
	int		size;
	int		rsize;
	char	*chars;
	char	*render;
}			t_editor_row;

struct editor_config
{
	int				cx, cy;
	int				rx;
	int				row_off;
	int				col_off;
	int				screen_rows;
	int				screen_cols;
	int				num_rows;
	t_editor_row	*row;
	char			*filename;
	char			statusmsg[80];
	time_t			statusmsg_time;
	struct termios	orig_termios;
};

struct editor_config E;

/******************************************************************************/// terminal

void	die(const char *str)
{
	write(STDOUT, "\x1b[2J", 4);
	write(STDOUT, "\x1b[H", 3);
	
	perror(str);
	exit(1);
}

void	disable_raw_mode(void)
{
	if (tcsetattr(STDIN, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
}

void	enable_raw_mode(void)
{
	if (tcgetattr(STDIN, &E.orig_termios) == -1)
		die("tcgetattr");
	atexit(disable_raw_mode);

	struct termios	raw = E.orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
}

int	editor_read_key(void)
{
	int		nread;
	char	c;
	
	while ((nread = read(STDIN, &c, 1)) != 1)
	{
		if (nread == -1 && errno != EAGAIN)
			die("read");
	}

	if (c == '\x1b')
	{
		char	seq[3];

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
	{
		return (c);
	}
}

int	get_cursor_position(int *rows, int *cols)
{
	char			buf[32];
	unsigned int	i = 0;

	if (write(STDOUT, "\x1b[6n", 4) != 4)
		return (-1);

	while (i < sizeof(buf) - 1)
	{
		if (read(STDIN, &buf[i], 1) != 1)
			break ;
		if (buf[i] == 'R')
			break ;
		i++;
	}
	buf[i] = '\0';

	
	if (buf[0] != '\x1b' || buf[1] != '[')
		return (-1);
	if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
		return (-1);
	return (0);
}

int	get_window_size(int *rows, int *cols)
{
	struct winsize	ws;

	if (ioctl(STDOUT, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		if (write(STDOUT, "\x1b[999C\x1b[999B", 12) != 12)
			return (-1);
		return (get_cursor_position(rows, cols));
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return (0);
	}
}

/******************************************************************************/// row operations

int	editor_row_cx_to_rx(t_editor_row *row, int cx)
{
	int	rx = 0;
	int	j = 0;
	while (j < cx)
	{
		if (row->chars[j] == '\t')
			rx += (KILO_TAB_STOP - 1) - (rx % KILO_TAB_STOP);
		rx++;
		j++;
	}
	return (rx);
}

void	editor_update_row(t_editor_row *row)
{
	int	tabs = 0;
	int	j;
	j = 0;
	while (j < row->size)
	{
		if (row->chars[j] == '\t')
			tabs++;
		j++;
	}

	free(row->render);
	row->render = malloc(row->size + tabs*(KILO_TAB_STOP - 1) + 1);

	int	idx = 0;
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

void	editor_append_row(char *s, size_t len)
{
	E.row = realloc(E.row, sizeof(t_editor_row) * (E.num_rows + 1));

	int	at = E.num_rows;
	E.row[at].size = len;
	E.row[at].chars = malloc(len + 1);
	memcpy(E.row[at].chars, s, len);
	E.row[at].chars[len] = '\0';

	E.row[at].rsize = 0;
	E.row[at].render = NULL;
	editor_update_row(&E.row[at]);

	E.num_rows++;
}


/******************************************************************************/// file i/o

void	editor_open(char *filename)
{
	free(E.filename);
	E.filename = strdup(filename);

	FILE	*fp = fopen(filename, "r");
	if (!fp)
		die("fopen");

	char	*line = NULL;
	size_t	linecap = 0;
	ssize_t	linelen;
	while ((linelen = getline(&line, &linecap, fp)) != -1)
	{
		while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen -1] == '\r'))
		{
			linelen--;
		}
		editor_append_row(line, linelen);
	}
	free(line);
	fclose(fp);
}

/******************************************************************************/// append buffer

struct abuf
{
	char	*b;
	int		len;
};

#define ABUF_INIT {NULL, 0}

void	ab_append(struct abuf *ab, const char *s, int len)
{
	char	*new = realloc(ab->b, ab->len + len);

	if (new == NULL)
		return ;
	memcpy(&new[ab->len], s, len);
	ab->b = new;
	ab->len += len;
}

void	ab_free(struct abuf *ab)
{
	free(ab->b);
}


/******************************************************************************/// output

void	editor_scroll(void)
{
	E.rx = 0;
	if (E.cy < E.num_rows)
	{
		E.rx = editor_row_cx_to_rx(&E.row[E.cy], E.cx);
	}

	if (E.cy < E.row_off)
	{
		E.row_off = E.cy;
	}
	if (E.cy >= E.row_off + E.screen_rows)
	{
		E.row_off = E.cy - E.screen_rows + 1;
	}
	if (E.rx < E.col_off)
	{
		E.col_off = E.rx;
	}
	if (E.rx >= E.col_off + E.screen_cols)
	{
		E.col_off = E.rx - E.screen_cols + 1;
	}
}

static void	welcome_msg(struct abuf *ab)
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


void	editor_draw_rows(struct abuf *ab)
{
	int	y = 0;

	while (y < E.screen_rows)
	{
		int	filerow = y + E.row_off;
		if (filerow >= E.num_rows)
		{
			if (E.num_rows == 0 && y == E.screen_rows / 3)
				welcome_msg(ab);
			else
				ab_append(ab, "~", 1);
		}
		else
		{
			int	len = E.row[filerow].rsize - E.col_off;
			if (len < 0)
				len = 0;
			if (len > E.screen_cols)
				len = E.screen_cols;
			ab_append(ab, &E.row[filerow].render[E.col_off], len);
		}
		
		ab_append(ab, "\x1b[K", 3);
		ab_append(ab, "\r\n", 2);
		y++;
	}
}

void	editor_draw_status_bar(struct abuf *ab)
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
	rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.num_rows);
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
	ab_append(ab, "\r\n", 2);
}

void	editor_draw_message_bar(struct abuf *ab)
{
	int	msglen;

	ab_append(ab, "\x1b[K", 3);
	msglen = strlen(E.statusmsg);
	if (msglen > E.screen_cols)
		msglen = E.screen_cols;
	if (msglen && time(NULL) - E.statusmsg_time < 5)
		ab_append(ab, E.statusmsg, msglen);
}

void	editor_refresh_screen(void)
{
	editor_scroll();

	struct abuf	ab = ABUF_INIT;

	ab_append(&ab, "\x1b[?25l", 6);
	ab_append(&ab, "\x1b[H", 3);

	editor_draw_rows(&ab);
	editor_draw_status_bar(&ab);
	editor_draw_message_bar(&ab);

	char	buf[32];
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.row_off) + 1, (E.rx - E.col_off) + 1);
	ab_append(&ab, buf, strlen(buf));

	ab_append(&ab, "\x1b[?25h", 6);

	write(STDOUT, ab.b, ab.len);
	ab_free(&ab);
}

void	editor_set_status_message(const char *fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);
	vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
	va_end(ap);
	E.statusmsg_time = time(NULL);
}

/******************************************************************************/// input

void	editor_move_cursor(int key)
{
	t_editor_row	*row;
	if (E.cy >= E.num_rows)
		row = NULL;
	else
		row = &E.row[E.cy];


	switch (key)
	{
		case ARROW_LEFT:
			if (E.cx != 0)
			{
				E.cx--;
			}
			else if (E.cy > 0)
			{
				E.cy--;
				E.cx = E.row[E.cy].size;
			}
			break ;
		case ARROW_RIGHT:
			if (row && E.cx < row->size)
			{
				E.cx++;
			}
			else if (row && E.cx == row->size)
			{
				E.cy++;
				E.cx = 0;
			}
			break ;
		case ARROW_UP:
			if (E.cy != 0)
			{
				E.cy--;
			}
			break ;
		case ARROW_DOWN:
			if (E.cy < E.num_rows)
			{
				E.cy++;
			}
			break ;
	}

	if (E.cy >= E.num_rows)
		row = NULL;
	else
		row = &E.row[E.cy];
	int	rowlen;
	if (row)
		rowlen = row->size;
	else
		rowlen = 0;
	if (E.cx > rowlen)
	{
		E.cx = rowlen;
	}
}

void	editor_process_keypress(void)
{
	int	c = editor_read_key();

	switch (c)
	{
		case CTRL_KEY('q'):
			write(STDOUT, "\x1b[2J", 4);
			write(STDOUT, "\x1b[H", 3);
			exit(0);
			break ;
		
		case HOME_KEY:
			E.cx = 0;
			break ;
		
		case END_KEY:
			if (E.cy < E.num_rows)
				E.cx = E.row[E.cy].size;
			break ;
		
		case PAGE_UP:
		case PAGE_DOWN:
		{
			if (c == PAGE_UP)
			{
				E.cy = E.row_off;
			}
			else if (c == PAGE_DOWN)
			{
				E.cy = E.row_off + E.screen_rows - 1;
				if (E.cy > E.num_rows)
					E.cy = E.num_rows;
			}

			int	times = E.screen_rows;
			while (times--)
			{
				if (c == PAGE_UP)
					editor_move_cursor(ARROW_UP);
				else
					editor_move_cursor(ARROW_DOWN);
			}
			break ;
		}
		
		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			editor_move_cursor(c);
			break ;
	}
}

/******************************************************************************/// init

void	init_editor(void)
{
	E.cx = 0;
	E.cy = 0;
	E.rx = 0;
	E.row_off = 0;
	E.col_off = 0;
	E.num_rows = 0;
	E.row = NULL;
	E.filename = NULL;
	E.statusmsg[0] = '\0';
	E.statusmsg_time = 0;

	if (get_window_size(&E.screen_rows, &E.screen_cols) == -1)
		die("get_window_size");
	E.screen_rows -= 2;
}

int	main(int argc, char *argv[])
{
	enable_raw_mode();
	init_editor();
	if (argc >= 2)
	{
		editor_open(argv[1]);
	}

	editor_set_status_message("HELP: Ctrl-Q = quit");

	while (1)
	{
		editor_refresh_screen();
		editor_process_keypress();
	}

	return (0);
}

