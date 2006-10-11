#ifndef _PORTHELP_H_
#define _PORTHELP_H_

#include <stdarg.h>
#include <stdio.h>

#include "debug.h"
#include "vstring.h"

extern int		 errno;

/* implementation of asprintf() for OSs which don't include it */
int		 my_asprintf(char **ret, char *format, ...);

/* skip over comments */
void	 skip_comment(FILE *fp);

#endif /* _PORTHELP_H_ */
