
#ifndef TERMINAL_H
# define TERMINAL_H

# include <termios.h>
# include <unistd.h>
# include <stdlib.h> // atexit
# include "utils.h"

struct s_editor_config
{
	int				screen_rows;
	int				screen_cols;
	struct termios	orig_termios;
};

extern struct s_editor_config	g_editor;

void	enable_raw_mode(void);

#endif // TERMINAL_H
