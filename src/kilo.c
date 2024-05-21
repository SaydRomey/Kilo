
#include "kilo.h"

// NEXT: https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html



int	main(void)
{
	enable_raw_mode();
	while (true)
	{
		editor_refresh_screen();
		editor_process_keypress();
	}

	return (0);
}
