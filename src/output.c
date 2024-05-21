
#include "kilo.h"

void	editor_refresh_screen(void)
{
	ft_putstr(CLEAR_SCREEN);
	ft_putstr(CURSOR_TOP_LEFT);

	editor_draw_rows();

	ft_putstr(CURSOR_TOP_LEFT);
}

void	editor_draw_rows(void)
{
	int	y;

	y = 0;
	while (y < g_editor.screen_rows)
	{
		ft_putstr("~\r\n");
		y++;
	}
}
