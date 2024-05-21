
#include "kilo.h"

/* terminal */

char	editor_read_key(void)
{
	int		nread;
	char	c;

	while ((nread = read(STDIN, &c, 1)) != 1)
	{
		if (nread == ERROR && errno != EAGAIN)
			exit_error("read");
	}
	return (c);
}

/* input */

void	editor_process_keypress(void)
{
	char	c;

	c = editor_read_key();
	switch (c)
	{
		case CTRL_KEY('q'):
			ft_putstr(CLEAR_SCREEN);
			ft_putstr(CURSOR_TOP_LEFT);
			exit(0);
			break;
	}
}
