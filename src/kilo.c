
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
	int				dirty;
	char			*filename;
	char			statusmsg[80];
	time_t			statusmsg_time;
	struct termios	orig_termios;
};

struct editor_config E;

/******************************************************************************/// prototypes

void	editor_set_status_message(const char *fmt, ...);
void	editor_refresh_screen(void);
char	*editor_prompt(char *prompt);


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

void	editor_insert_row(int at, char *s, size_t len)
{
	if (at < 0 || at > E.num_rows)
		return ;

	E.row = realloc(E.row, sizeof(t_editor_row) * (E.num_rows + 1));
	memmove(&E.row[at + 1], &E.row[at], sizeof(t_editor_row) * (E.num_rows - at));

	E.row[at].size = len;
	E.row[at].chars = malloc(len + 1);
	memcpy(E.row[at].chars, s, len);
	E.row[at].chars[len] = '\0';

	E.row[at].rsize = 0;
	E.row[at].render = NULL;
	editor_update_row(&E.row[at]);

	E.num_rows++;
	E.dirty++;
}

void	editor_free_row(t_editor_row *row)
{
	free(row->render);
	free(row->chars);
}

void	editor_del_row(int at)
{
	if (at < 0 || at >= E.num_rows)
		return ;
	editor_free_row(&E.row[at]);
	memmove(&E.row[at], &E.row[at + 1], sizeof(t_editor_row) * (E.num_rows - at - 1));
	E.num_rows--;
	E.dirty++;
}

void	editor_row_insert_char(t_editor_row *row, int at, int c)
{
	if (at < 0 || at > row->size)
		at = row->size;
	row->chars = realloc(row->chars, row->size + 2);
	memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
	row->size++;
	row->chars[at] = c;
	editor_update_row(row);
	E.dirty++;
}

void	editor_row_append_string(t_editor_row *row, char *s, size_t len)
{
	row->chars = realloc(row->chars, row->size + len + 1);
	memcpy(&row->chars[row->size], s, len);
	row->size += len;
	row->chars[row->size] = '\0';
	editor_update_row(row);
	E.dirty++;
}

void	editor_row_del_char(t_editor_row *row, int at)
{
	if (at < 0 || at >= row->size)
		return ;
	memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
	row->size--;
	editor_update_row(row);
	E.dirty++;
}

/******************************************************************************/// editor operations

void	editor_insert_char(int c)
{
	if (E.cy == E.num_rows)
	{
		editor_insert_row(E.num_rows, "", 0);
	}
	editor_row_insert_char(&E.row[E.cy], E.cx, c);
	E.cx++;
}

void	editor_insert_new_line(void)
{
	t_editor_row *row;

	if (E.cx == 0)
	{
		editor_insert_row(E.cy, "", 0);
	}
	else
	{
		row = &E.row[E.cy];
		editor_insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
		row = &E.row[E.cy];
		row->size = E.cx;
		row->chars[row->size] = '\0';
		editor_update_row(row);
	}
	E.cy++;
	E.cx = 0;
}

void	editor_del_char(void)
{
	t_editor_row *row;

	if (E.cy == E.num_rows)
		return ;
	if (E.cx == 0 && E.cy == 0)
		return ;

	row = &E.row[E.cy];
	if (E.cx > 0)
	{
		editor_row_del_char(row, E.cx - 1);
		E.cx--;
	}
	else
	{
		E.cx = E.row[E.cy - 1].size;
		editor_row_append_string(&E.row[E.cy - 1], row->chars, row->size);
		editor_del_row(E.cy);
		E.cy--;
	}
}


/******************************************************************************/// file i/o

char	*editor_rows_to_string(int *buflen)
{
	int	totlen = 0;
	int	j;

	j = 0;
	while (j < E.num_rows)
	{
		totlen += E.row[j].size + 1;
		j++;
	}
	*buflen = totlen;

	char	*buf = malloc(totlen);
	char	*p = buf;
	j = 0;
	while (j < E.num_rows)
	{
		memcpy(p, E.row[j].chars, E.row[j].size);
		p += E.row[j].size;
		*p = '\n';
		p++;
		j++;
	}
	return (buf);
}

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
		editor_insert_row(E.num_rows, line, linelen);
	}
	free(line);
	fclose(fp);
	E.dirty = 0;
}

void	editor_save(void)
{
	if (E.filename == NULL)
	{
		E.filename = editor_prompt("Save as: %s (ESC) to cancel");
	}
	if (E.filename == NULL)
	{
		editor_set_status_message("Save aborted");
		return ;
	}

	int		len;
	char	*buf = editor_rows_to_string(&len);

	int		fd = open(E.filename, O_RDWR | O_CREAT, 0644);
	if (fd != -1)
	{
		if (ftruncate(fd, len) != -1)
		{
			if (write(fd, buf, len) == len)
			{
				close(fd);
				free(buf);
				E.dirty = 0;
				editor_set_status_message("%d bytes written to disk", len);
				return ;
			}
		}
		close(fd);
	}
	free(buf);
	editor_set_status_message("Can't save! I/O error: %s", strerror(errno));
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
	char	*dirty;
	int		len;
	int		rlen;

	ab_append(ab, "\x1b[7m", 4);
	if (E.filename)
		filename = E.filename;
	else
		filename = "[No Name]";
	if (E.dirty)
		dirty = "(modified)";
	else
		dirty = "";
	len = snprintf(status, sizeof(status), "%.20s - %d lines %s", filename, E.num_rows, dirty);
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

char	*editor_prompt(char *prompt)
{
	size_t	bufsize = 128;
	char	*buf = malloc(bufsize);

	size_t	buflen = 0;
	buf[0] = '\0';

	while (1)
	{
		editor_set_status_message(prompt, buf);
		editor_refresh_screen();

		int	c = editor_read_key();
		if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE)
		{
			if (buflen != 0)
				buf[--buflen] = '\0';
		}
		else if (c == '\x1b')
		{
			editor_set_status_message("");
			free(buf);
			return (NULL);
		}
		else if (c == '\r')
		{
			if (buflen != 0)
			{
				editor_set_status_message("");
				return (buf);
			}
			else if (!iscntrl(c) && c < 128)
			{
				if (buflen == bufsize - 1)
				{
					bufsize *= 2;
					buf = realloc(buf, bufsize);
				}
				buf[buflen++] = c;
				buf[buflen] = '\0';
			}
		}
	}
}

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
	static int	quit_times = KILO_QUIT_TIMES;

	int	c = editor_read_key();

	switch (c)
	{
		case '\r':
			editor_insert_new_line();
			break;
		
		case CTRL_KEY('q'):
			if (E.dirty && quit_times > 0)
			{
				editor_set_status_message("WARNING!!! File has unsaved changes. ""Press CTRL-! %d more times to quit.", quit_times);
				quit_times--;
				return ;
			}
			write(STDOUT, "\x1b[2J", 4);
			write(STDOUT, "\x1b[H", 3);
			exit(0);
			break ;
		
		case CTRL_KEY('s'):
			editor_save();
			break ;
		
		case HOME_KEY:
			E.cx = 0;
			break ;
		
		case END_KEY:
			if (E.cy < E.num_rows)
				E.cx = E.row[E.cy].size;
			break ;
		
		case BACKSPACE:
		case CTRL_KEY('h'):
		case DEL_KEY:
			if (c == DEL_KEY)
				editor_move_cursor(ARROW_RIGHT);
			editor_del_char();
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
		
		case CTRL_KEY('l'):
		case '\x1b':
			break ;
		
		default:
			editor_insert_char(c);
			break ;
	}
	quit_times = KILO_QUIT_TIMES;
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
	E.dirty = 0;
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

	editor_set_status_message("HELP: Ctrl-S = save | Ctrl-Q = quit");

	while (1)
	{
		editor_refresh_screen();
		editor_process_keypress();
	}

	return (0);
}

