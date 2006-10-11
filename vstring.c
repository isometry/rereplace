#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "vstring.h"

#define VSTRSPACE(x)	((x)->size - (x)->length - 1)
#define	ZEROSPACE(x)	(memset((x)->string + (x)->length, 0, (x)->size - (x)->length))

extern int snprintf(char *, size_t, const char *, ...);

vstring *
vstrnew(char *istr)
{
	vstring	*vstr;

	debug("* vstrnew()\n");

	if ((vstr = malloc(sizeof(vstring))) == NULL)
		fault(errno, "  malloc(%d)\n", sizeof(vstring));

	/* doesn't count terminating '\0' */
	vstr->length = (istr == NULL || *istr == '\0') ? 0 : strlen(istr);

	for (vstr->size  = VSTR_MIN;
		 vstr->size <= vstr->length; /* room for trailing '\0' */
		 vstr->size += VSTR_INC)
		/* nada */;

	if ((vstr->string = malloc(vstr->size)) == NULL)
		fault(errno, "  malloc(%d)\n", vstr->size);

	if (istr != NULL && *istr != '\0')
		memcpy(vstr->string, istr, vstr->length);

	ZEROSPACE(vstr);

	debug("  created vstring: string = '%s'; length = %d; size = %d\n",
			CSTR(vstr), VSTRLEN(vstr), vstr->size);

	return vstr;
}

void
vstrrst(vstring **vstr)
{
	char	*cp;

	debug("* vstrrst('%s')\n", CSTR(*vstr));

	if (*vstr == NULL)
	{
		debug("  NULL vstring\n");
		return;
	}

	if ((*vstr)->size > VSTR_MIN)
	{
		debug("  shrinking\n");

		(*vstr)->size -= VSTR_INC;

		if ((cp = realloc((*vstr)->string, (*vstr)->size)) == NULL)
			fault(errno, "realloc(vstr->string, %d)\n", (*vstr)->size);

		(*vstr)->string = cp;
	}

	(*vstr)->length = 0;
	ZEROSPACE(*vstr);
}

void
vstrdel(vstring **vstr)
{
	debug("* vstrdel(): ");

	if (*vstr == NULL)
	{
		debug("NULL vstring\n");
		return;
	}

	debug("'%s'\n", CSTR(*vstr));

	free(CSTR(*vstr));
	free(*vstr);
	*vstr = NULL;
}

int
vstrspace(vstring *vstr, size_t space)
{
	char	*cp;

#ifdef VDEBUG
	debug("* vstrspace('%s', %d)\n", CSTR(vstr), space);
#endif

	if (VSTRSPACE(vstr) >= space)
	{
#ifdef VDEBUG
		debug("  no growth: length = %d, size = %d, space = %d\n", vstr->length, vstr->size, VSTRSPACE(vstr));
#endif
		return ((VSTRSPACE(vstr) == space) ? 0 : +1);
	}

#ifdef VDEBUG
	debug("  growing...\n");
#endif

	while (VSTRSPACE(vstr) < space)
		vstr->size += VSTR_INC;

	if ((cp = realloc(vstr->string, vstr->size)) == NULL)
		fault(errno, "realloc(vstr->string, %d)\n", vstr->size);

	vstr->string = cp;

	ZEROSPACE(vstr);

#ifdef VDEBUG
	debug("  vstr->size = %d\n", vstr->size);
#endif
	return -1;
}

vstring *
vstrncat(vstring *vstr, const char *append, size_t count)
{
	debug("* vstrncat('%s', '%s', %d)\n", CSTR(vstr), append, count);

	vstrspace(vstr, count);

	strncat(vstr->string, append, count);
	vstr->length += count;

	return vstr;
}

vstring *
vstrputc(vstring *vstr, int append)
{
#ifdef VDEBUG
	debug("* vstrputc('%c')\n", append);
#endif

	vstrspace(vstr, 1);
	vstr->string[vstr->length++] = append;

	return vstr;
}

vstring *
vstrcatv(vstring *vstr1, const vstring *vstr2)
{
	debug("* vstrcatv('%s', '%s')\n", CSTR(vstr1), CSTR(vstr2));

	vstrspace(vstr1, vstr2->length);

	strcat(vstr1->string, vstr2->string);
	vstr1->length += vstr2->length;

	return vstr1;
}

size_t
vstrread(int d, vstring *vstr, size_t nbytes)
{
	size_t	rbytes;

	debug("* vstrread(%d, '%s', %d)\n", d, CSTR(vstr), nbytes);

	vstrspace(vstr, nbytes);
	rbytes = read(d, vstr->string + vstr->length, nbytes);
	vstr->length += rbytes;

	debug("  read %d bytes\n", rbytes);

	return rbytes;
}

#define ISHTMLSAFE(x)	(isalnum(x) || (x) == '.' || (x) == '/' || (x) == ':')

vstring *
vstresc(char *istr)
{
	char	*icp,
			*ecp;
	char	 tcp[4];
	vstring	*ostr;
	size_t	 span = 0;

	debug("* vstresc('%s')\n", istr);

	icp = istr;
	ecp = icp + strlen(istr) - 1;
	ostr = vstrnew("");

	for (;;)
	{
		span = 0;

		while (ISHTMLSAFE((int)icp[span]))
			++span;

		vstrncat(ostr, icp, span);
		icp += span;

		if (*icp == '\0' || ( *icp == '\n' && icp == ecp))
		{
			debug("  EOL\n");
			break;
		}

		snprintf(tcp, 4, "%%%.2X", (int)*icp++);
		vstrncat(ostr, tcp, 4);
	}

	return ostr;
}

vstring *
vstrfgetln(FILE *stream)
{
	vstring	*lstr;
	int		 ch;

	debug("* vstrfgetln\n");

	if (feof(stream))
	{
		debug("  EOF\n");
		return NULL;
	}

	lstr = vstrnew("");

	while (EOF != (ch = getc(stream)))
	{
		if (ch == '\n')
			break;
		else
			vstrputc(lstr, ch);
	}

	debug("  line: '%s'\n", CSTR(lstr));

	return lstr;
}
