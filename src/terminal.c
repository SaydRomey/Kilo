
#include "terminal.h"

struct termios	g_orig_termios; // Definition

static void	disable_raw_mode(void)
{
	if (tcsetattr(STDIN, TCSAFLUSH, &g_orig_termios) == FAILURE)
		exit_error("tcsetattr");
}

static void	set_termios_flags(struct termios *raw)
{
	raw->c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw->c_oflag &= ~(OPOST);
	raw->c_cflag |= (CS8);
	raw->c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
}

static void	set_read_timeout(struct termios *raw)
{
	raw->c_cc[VMIN] = 0;
	raw->c_cc[VTIME] = 1;
}

void	enable_raw_mode(void)
{
	struct termios	raw;

	if (tcgetattr(STDIN, &g_orig_termios) == FAILURE)
		exit_error("tcgetattr");
	atexit(disable_raw_mode);

	raw = g_orig_termios;
	set_termios_flags(&raw);
	set_read_timeout(&raw);

	if (tcsetattr(STDIN, TCSAFLUSH, &raw) == FAILURE)
		exit_error("tcsetattr");
}
