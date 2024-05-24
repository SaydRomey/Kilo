
#ifndef KILO_H
# define KILO_H

# include <termios.h>
# include <stdbool.h>
# include <errno.h>
# include <ctype.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>

# include "utils.h"

# define KILO_VERSION "0.0.1"
# define CTRL_KEY(k) ((k) & 0x1f)

struct	s_abuf
{
	char	*buffer;
	int		len;
};

# define ABUF_INIT	{NULL, 0}

struct s_editor_config
{
	int				screen_rows;
	int				screen_cols;
	struct termios	orig_termios;
};

extern struct s_editor_config	g_editor;

// append_buffer.c
void	ab_append(struct s_abuf *ab, const char *str, int len);
void	ab_free(struct s_abuf *ab);

// input.c
char	editor_read_key(void);
void	editor_process_keypress(void);

// misc.c
void	print_ascii(char c);

// output.c
void	editor_refresh_screen(void);
void	editor_draw_rows(struct s_abuf *ab);

// terminal.c
void	enable_raw_mode(void);

#endif // KILO_H