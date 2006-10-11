#include <sys/time.h>
#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "main.h"
#include "porthelp.h"
#include "tuple.h"
#include "vstring.h"

extern char	*strdup(const char *);

static int	fill_map(tuple **head, char *mapfile);
static void	free_map(tuplemap *curmap);

/* check each tuplemap entry against input.
 * if we get a match, shunt the entry toward the top,
 * increasing the likelihood of a quick match next time.
 */
tuple *
match_tuple(tuplemap *curmap, vstring *input,
		size_t nmatch, regmatch_t regmatch[], int reorder)
{
	tuple		*s2,
				*s1,
				*te;

	debug("* match_tuple()\n");

	for (s2 = NULL, s1 = NULL, te = curmap->head_tuple;
		 te != NULL;
		 s2 = s1, s1 = te, te = te->next)
	{
		debug("  '%s' =~ '%s': ", CSTR(input), te->pattern);

		switch (regexec(&(te->re), CSTR(input), nmatch, regmatch, 0))
		{
			case 0:
				debug("match\n");

				if (reorder == 0)
				{
					if (s1 != NULL)
					{
						if (s2 != NULL)
							s2->next = te;
						else
							curmap->head_tuple = te;

						s1->next = te->next;
						te->next = s1;
						debug("REORDERED\n");
					}
				}

				if (strcmp(te->replace, "-") == 0)
					return NULL;

				return te;

			case REG_NOMATCH:
				debug("no match\n");
				break;

			default:
				fault(1, "unexpected error with regexec()\n");
		}

	}

	debug("  no match found\n");
	return NULL;
}

/* check a tuple's mtime against its corresponding file */
int
check_map(tuplemap *curmap)
{
	static int	check = 0;
	struct stat	fs_stat;

	debug("* check_map(): ");

	if ((check = (check + 1) % CHECK_FREQ) == 0)
	{
		debug("doing check\n");

		if (stat(curmap->filename, &fs_stat) != 0)
			fault(4, "failed to stat('%s'): %s\n",
					curmap->filename, strerror(errno));

		if (curmap->mtime == fs_stat.st_mtime)
		{
			debug("  file unchanged\n");
			return 0;
		}
		else
		{
			debug("  file changed\n");
			return 1;
		}
	}
	else
	{
		debug("skipping check\n");
		return 0;
	}
}

/* update a tuplemap that is out of date */
void
update_map(tuplemap *curmap)
{
	struct stat	fs_stat;

	debug("* update_map()\n");

	if (stat(curmap->filename, &fs_stat) != 0)
		fault(5, "failed to stat('%s'): %s\n",
				curmap->filename, strerror(errno));

	curmap->mtime = fs_stat.st_mtime;
	debug("  current map = %s\n", curmap->filename);
	debug("  current mtime = %ld\n", (long)curmap->mtime);

	free_map(curmap);
	
	if (fill_map(&(curmap->head_tuple), curmap->filename) != 0)
		fault(6, "failed to fill map from '%s'\n", curmap->filename);
}

/* read a file into a linked list of tuples */
static int
fill_map(tuple **head, char *mapfile)
{
	FILE	*fs;
	vstring	*fileline = NULL,
			*pattern  = NULL,
			*replace  = NULL;
	char	*cp;
	size_t	 span;
	tuple	*this = NULL;
	tuple	*tail = NULL;

	debug("* fill_map()\n");

	pattern = vstrnew("");
	replace = vstrnew("");

	if ((fs = fopen(mapfile, "r")) == NULL)
		fault(7, "map file '%s' does not exist\n", mapfile);

	debug("  opened mapfile\n");

	while (!feof(fs))
	{
		/* could this leak if feof(fs) before vstrfgetln(fs) returns NULL? */
		vstrdel(&fileline);
		vstrrst(&pattern);
		vstrrst(&replace);

		skip_comment(fs);

		if ((fileline = vstrfgetln(fs)) == NULL)
			break;

		debug("  fileline: '%s'\n", CSTR(fileline));

		cp = CSTR(fileline);
		span = strcspn(cp, WHITESPACE);
		vstrncat(pattern, cp, span);
		cp += span;
		cp += strspn(cp, WHITESPACE);
		span = strcspn(cp, WHITESPACE);
		vstrncat(replace, cp, span);

		debug("  pattern = '%s'; replace = '%s'\n",
				CSTR(pattern), CSTR(replace));

		if ((this = malloc(sizeof(tuple))) == NULL)
			fault(errno, "malloc(%d)\n", sizeof(tuple));


		this->pattern = strdup(CSTR(pattern));
		this->replace = strdup(CSTR(replace));
		/* associated vstring memory is reset on the next pass */

		/* compile regular expressions */
		if (regcomp(&(this->re), this->pattern, REG_EXTENDED) != 0)
			fault(8, "problem compiling '%s'\n", this->pattern);
		else
			debug("  compiled '%s'\n", this->pattern);

		debug("  this->pattern = '%s'; this->replace = '%s'\n",
				this->pattern, this->replace);

		this->next = NULL;

		if (*head == NULL)
			*head = this;
		else
			tail->next = this;

		tail = this;
	}

	debug("  finished reading file\n");

	fclose(fs);

	vstrdel(&pattern);
	vstrdel(&replace);

	debug("  finished reading configuration\n");

	return 0;

}

/* free memory from a linked list */
static void
free_map(tuplemap *curmap)
{
	tuple	*this;

	debug("* free_map()\n");

	if (curmap->head_tuple == NULL)
	{
		debug("  empty map\n");
		return;
	}

	while ((this = curmap->head_tuple) != NULL)
	{
		curmap->head_tuple = this->next;
		debug("  freeing: '%s'\n", this->replace);
		free(this->pattern);
		free(this->replace);
		free(this);
	}
}

/* print the tuplemap for debugging */
void
print_map(tuplemap *curmap)
{
	tuple	*this;
	int		 num = 0;

	debug("* print_map(): file = '%s', mtime = %ld\n",
			curmap->filename, curmap->mtime);

	if (curmap->head_tuple == NULL)
		printf("empty map\n");

	for (this = curmap->head_tuple;
		 this != NULL;
		 this = this->next)
	{
		printf("%.2d: %-32s -> '%s'\n", ++num, this->pattern, this->replace);
	}
}

void
expand_replace(vstring *istr /* input */, char *rstring /* replace */,
		vstring *ostr /* output */, size_t nmatch, regmatch_t *regmatch)
{
	char	*rcp,
			*icp;
	int		 n;
	size_t	 span;

	debug("* expand_replace()\n");

	rcp = rstring;
	vstrrst(&ostr);

	for (;;)
	{
		span = strcspn(rcp, "$");
		debug("  span: %d\n", span);
		vstrncat(ostr, rcp, span);
		rcp += span;
		debug("  ostr: '%s'\n", CSTR(ostr));

		/* have we reached the end of the string? */
		if (*rcp == '\0')
			break;

		if (isdigit((int)rcp[1]))
		{
			debug("  processing reference to backrefence $%c\n",
					rcp[1]);
			n = rcp[1] - '0';	/* get the backreference number */ 
			if ((n <= nmatch) && (regmatch[n].rm_eo >= regmatch[n].rm_so))
			{
				span = regmatch[n].rm_eo - regmatch[n].rm_so;
				debug("  backreference is %d characters long,"
						"starting at %d\n",
						span, regmatch[n].rm_so);

				icp = CSTR(istr) + regmatch[n].rm_so;
				vstrncat(ostr, icp, span);
				icp += span;
				rcp += 2;
			}
			else
				fault(10, "invalid backreference label: '%c%c'\n",
						rcp[0], rcp[1]);
		}
		else if (rcp[1] == '\0')
		{
			vstrncat(ostr, rcp, 1);
			rcp += 1;
		}
		else
		{
			vstrncat(ostr, rcp, 2);
			rcp += 2;
		}

	}
}

