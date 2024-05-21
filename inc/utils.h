
#ifndef UTILS_H
# define UTILS_H

# include <stdio.h> // perror
# include <errno.h>
# include <stdlib.h> // exit
# include <unistd.h> // write

# define STDIN		0
# define STDOUT		1
# define STDERR		2

# define SUCCESS	0
# define FAILURE	1
# define ERROR		-1

# define CLEAR_SCREEN		"\x1b[2J"
# define CURSOR_TOP_LEFT	"\x1b[H"

void	editor_refresh_screen(void);
void	exit_error(const char *str);

#endif // UTILS_H
