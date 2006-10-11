#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdarg.h>

/* output to stderr and exit(errnum) */
void fault(int errnum, char *format, ...);

/* output to stderr when DEBUG is defined */
void debug(char *format, ...);

#endif /* _DEBUG_H_ */
