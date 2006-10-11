#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "porthelp.h"
#include "vstring.h"

extern int vsnprintf(char *, size_t, const char *, va_list);

int
my_asprintf(char **ret, char *format, ...)
{
	va_list	ap;
	int		num;
	char	ch[1];

	debug("* my_asprintf()\n");

	va_start(ap, format);
	if ((num = vsnprintf(ch, 1, format, ap)) < 0)
		fault(1, "vsnprintf()\n");
	if ((*ret = malloc(num+1)) == NULL)
		fault(ENOMEM, "malloc()\n");
	(*ret)[num] = '\0';
	if ((num = vsnprintf(*ret, num+1, format, ap)) < 0)
		fault(1, "second vsnprintf()\n");
	va_end(ap);

	return num;
}

void
skip_comment(FILE *fp)
{
	int	ch;

	debug("* skip_comment()\n");

	while (EOF != (ch = getc(fp)))
	{
		/* skip leading blanks */
		while (ch == ' ' || ch == '\t')
			ch = getc(fp);

		if (ch == EOF)
			break;

		/* blank line or comment */
		if (ch != '\n' && ch != '#')
			break;

		/* skip over comment */
		while (ch != '\n' && ch != EOF)
			ch = getc(fp);
	}
	if (ch != EOF)
		ungetc(ch, fp);
}

