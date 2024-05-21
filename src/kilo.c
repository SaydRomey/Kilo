
#include "kilo.h"

// NEXT: https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html

static void	print_ascii(char c)
{
	if (iscntrl(c))
		printf("%d\r\n", c);
	else
		printf("%d ('%c')\r\n", c, c);
}

int	main(void)
{
	char	c;

	enable_raw_mode();
	while (true)
	{		
		c = '\0';
		if (read(STDIN, &c, 1) == FAILURE && errno != EAGAIN)
			exit_error("read");
		print_ascii(c);
		if (c == 'q')
			break ;
	}
	return (0);
}
