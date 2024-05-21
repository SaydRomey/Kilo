
#ifndef KILO_H
# define KILO_H

# include <stdbool.h>
# include <errno.h>
# include <ctype.h> // iscntrl
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

# include "utils.h"
# include "terminal.h"

# define CTRL_KEY(k) ((k) & 0x1f)

// input.c
char	editor_read_key(void);
void	editor_process_keypress(void);

// misc.c
void	print_ascii(char c);

// output.c
void	editor_refresh_screen(void);
void	editor_draw_rows(void);

#endif // KILO_H