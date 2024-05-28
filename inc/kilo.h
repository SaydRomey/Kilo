
#ifndef KILO_H
# define KILO_H

# define _DEFAULT_SOURCE
# define _BSD_SOURCE
# define _GNU_SOURCE

# include <ctype.h>
# include <errno.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ioctl.h>
# include <sys/types.h>
# include <termios.h>
# include <unistd.h>

# include "utils.h"

# define KILO_VERSION 	"0.0.1"
# define KILO_TAB_STOP	8

# define CTRL_KEY(k) ((k) & 0x1f)

enum	e_editor_read_key
{
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN
};

struct	s_abuf
{
	char	*buffer;
	int		len;
};

# define ABUF_INIT	{NULL, 0}

typedef struct s_editor_row
{
	int		size;
	int		rsize;
	char	*chars;
	char	*render;
}			t_editor_row;

struct s_editor_config
{
	t_point			cursor;
	int				render_x;
	int				row_offset;
	int				col_offset;
	int				screen_rows;
	int				screen_cols;
	int				num_rows;
	t_editor_row	*row;
	char			*filename;
	struct termios	orig_termios;
};

extern struct s_editor_config	E;

// append_buffer.c
void	ab_append(struct s_abuf *ab, const char *str, int len);
void	ab_free(struct s_abuf *ab);

// input.c
int		editor_read_key(void);
void	editor_process_keypress(void);

int		editor_row_cursorx_to_render_x(t_editor_row *row, int cursor_x);

// misc.c
void	print_ascii(char c);

// output.c
void	editor_draw_rows(struct s_abuf *ab);
void	editor_refresh_screen(void);

// terminal.c
void	enable_raw_mode(void);

// window_size.c
int		get_cursor_pos(int *rows, int *cols);
int		get_window_size(int *rows, int *cols);

#endif // KILO_H
