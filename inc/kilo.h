#ifndef KILO_H
# define KILO_H

# define _DEFAULT_SOURCE
# define _BSD_SOURCE
# define _GNU_SOURCE

# include <ctype.h>
# include <errno.h>
# include <fcntl.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdarg.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ioctl.h>
# include <sys/types.h>
# include <termios.h>
# include <time.h>
# include <unistd.h>

# define STDIN		0
# define STDOUT		1
# define STDERR		2

# define KILO_VERSION "0.0.1"
# define KILO_TAB_STOP 8

# define CTRL_KEY(k) ((k) & 0x1f)

enum editor_key
{
	BACKSPACE = 127,
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

#endif