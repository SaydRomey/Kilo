
#include "utils.h"

void	exit_error(const char *str)
{
	perror(str);
	exit(ERROR);
}
