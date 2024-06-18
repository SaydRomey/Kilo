
#include "utils.h"

size_t	ft_strlen(const char *str)
{
	size_t	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

size_t	ft_putstr(const char *str)
{
	return (write(STDOUT, str, ft_strlen(str)));
}

void	exit_error(const char *str)
{
	ft_putstr(CLEAR_SCREEN);
	ft_putstr(CURSOR_TOP_LEFT);
	perror(str);
	exit(FAILURE);
}
