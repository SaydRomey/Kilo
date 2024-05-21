
#ifndef UTILS_H
# define UTILS_H

# include <stdio.h> // perror
# include <errno.h>
# include <stdlib.h> // exit

# define STDIN		0
# define STDOUT		1
# define STDERR		2

# define SUCCESS	0
# define ERROR		1
# define FAILURE	-1

void	exit_error(const char *str);

#endif // UTILS_H
