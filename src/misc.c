
#include "kilo.h"

void	print_ascii(char c)
{
	if (iscntrl(c))
		printf("%d\r\n", c);
	else
		printf("%d ('%c')\r\n", c, c);
}
