
#ifndef TERMINAL_H
# define TERMINAL_H

# include <termios.h>
# include <unistd.h>
# include <stdlib.h> // atexit
# include "utils.h"

extern struct termios	g_orig_termios; // Declaration only

void	enable_raw_mode(void);

#endif // TERMINAL_H
