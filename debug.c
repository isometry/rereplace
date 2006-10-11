#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"

/* output to stderr and exit(errnum) */
void
fault(int errnum, char *format, ...)
{
	va_list	ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	exit(errnum);
}

/* output to stderr when DEBUG is defined */
void
debug(char *format, ...)
{
#ifdef DEBUG
	va_list	ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
#endif 
}

