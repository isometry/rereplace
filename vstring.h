
#ifndef _VSTRING_H_
#define _VSTRING_H_

#include <stdio.h>

#define	VSTR_MIN	1024
#define	VSTR_INC	512

#define CSTR(x)			((x)->string)
#define VSTRLEN(x)		((x)->length)
#define VSTRSIZE(x)		((x)->size)

typedef struct vstring vstring;
struct vstring
{
	char	*string;	/* the string itself */
	size_t	 length;	/* the length of the string */
	size_t	 size;		/* the size of the strings buffer */
};

vstring	*vstrnew(char *istr);
void	 vstrrst(vstring **vstr);
void	 vstrdel(vstring **vstr);
int		 vstrspace(vstring *vstr, size_t space);
vstring	*vstrncat(vstring *vstr, const char *append, size_t count);
vstring	*vstrcatv(vstring *vstr1, const vstring *vstr2);
size_t	 vstrread(int d, vstring *vstr, size_t nbytes);
vstring	*vstresc(char *istr);
vstring	*vstrfgetln(FILE *stream);

#endif /* _VSTRING_H_ */
