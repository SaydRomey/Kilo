
#include "kilo.h"

void	ab_append(struct s_abuf *ab, const char *str, int len)
{
	char	*new;

	new = realloc(ab->buffer, ab->len + len);
	if (new == NULL)
		return ;
	memcpy(&new[ab->len], str, len);
	ab->buffer = new;
	ab->len += len;
}

void	ab_free(struct s_abuf *ab)
{
	free(ab->buffer);
}
