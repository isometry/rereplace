#ifndef _TUPLE_H_
#define _TUPLE_H_

#include <sys/time.h>

#include <regex.h>

#include "vstring.h"

#define NO_MATCH	"NULL"
#define MAXSUB		10
#define CHECK_FREQ	10
#define	WHITESPACE	" \t"

typedef struct tuple tuple;
struct tuple
{
	tuple	*next;			/* next tuple */
	char	*pattern;
	regex_t	 re;			/* the compiled pattern to match */
	char	*replace;		/* the string to output */
};

typedef struct tuplemap tuplemap;
struct tuplemap
{
	char	*filename;		/* tuple map file */
	time_t	 mtime;			/* modification time of map file */
	tuple	*head_tuple;	/* head entry in current list */
};

/* match a given pattern against each pattern in a map and return the expanded string */
tuple *match_tuple(tuplemap *curmap, vstring *input,
		size_t nmatch, regmatch_t regmatch[], int reorder);

/* check a tuple's mtime against its corresponding file */
int	 check_map(tuplemap *curmap);

/* update a tuplemap that is out of date */
void update_map(tuplemap *curmap);

void print_map(tuplemap *curmap);

/* expand replacement string */ 
void expand_replace(vstring *istr /* input */, char *rstring /* replace */,
		vstring *ostr /* output */, size_t nmatch, regmatch_t *regmatch);

#endif /* _TUPLE_H_ */


