#include <sys/time.h>
#include <sys/types.h>

#include <errno.h>
#include <pwd.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "main.h"
#include "porthelp.h"
#include "tuple.h"
#include "vstring.h"

extern int getopt(int, char * const *, const char *);
extern char *optarg;
extern int optind, opterr, optopt;

int
main(int argc, char *argv[])
{
	tuplemap		 tm;
	tuple			*te;
	size_t			 nbytes;
	vstring			*input,
					*output,
					*tvstr;
	fd_set			 readfds;
	struct timeval	 tv;
	regmatch_t		 regmatch[MAXSUB];
	int				 ch;
	int				 fflag = 0,
					 iflag = 0,
					 pflag = 0;
	char			*cmd = NULL,
					*mapfile = NULL,
					*user = SETUID;

	debug("* main(): started as '%s'\n", argv[0]);

	cmd = argv[0];

	while ((ch = getopt(argc, argv, "fhim:pu:")) != -1)
		switch (ch)
		{
			case 'f':
				fflag = 1;
				break;
			case 'i':
				iflag = 1;
				break;
			case 'm':
				mapfile = optarg;
				break;
			case 'p':
				pflag = 1;
				break;
			case 'u':
				user = optarg;
				break;
			case 'h':
			default:
				fault(0, "USAGE: %s [-fhip] [-m file] [-u user]\n", cmd);
		}

	argc -= optind;
	argv += optind;

	/* lower privileges if we have them */
	if (geteuid() == (uid_t)0)
		drop_privs(user);

	/* initialise tuple map */
	if (mapfile == NULL)
		my_asprintf(&(tm.filename), "%s.map", cmd);
	else
		tm.filename = mapfile;

	tm.head_tuple = NULL;
	update_map(&tm);

	if (pflag == 1)
	{
		print_map(&tm);
		exit(0);
	}

	/* initialise input */
	input = vstrnew("");
	output = vstrnew("");

	if (iflag == 1)
		print_map(&tm);

	debug("-------- wait on select() --------\n");

	for (FD_ZERO(&readfds), FD_SET(STDIN_FILENO, &readfds);
		 select(1, &readfds, NULL, NULL, NULL) > 0;
		 FD_ZERO(&readfds), FD_SET(STDIN_FILENO, &readfds))
	{
		debug(" select() detected input\n");

		vstrrst(&input);
		vstrrst(&output);

		/* read in the whole of stdin */
		do
		{
			if ((nbytes = vstrread(STDIN_FILENO, input, INPUTCHUNK)) == 0)
			{
				debug("EOF\n");
				exit(0);
			}

			/* check the total amount read */
			if (VSTRLEN(input) > MAXINPUT)
				fault(1, "URL > %d characters!\n", MAXINPUT);

			/* the select() is required for the case that nbytes == INPUTCHUNK,
			 * but STDIN is empty... normally read() would block.
			 */
		} while (nbytes == INPUTCHUNK
					&& (FD_ZERO(&readfds), FD_SET(STDIN_FILENO, &readfds),
						tv.tv_sec = tv.tv_usec = 0L,
						select(1, &readfds, NULL, NULL, &tv)));

		/* escape the read data */
		tvstr = vstresc(CSTR(input));
		vstrdel(&input);
		input = tvstr;

		debug(" escaped data: '%s'\n", CSTR(input));

		if (check_map(&tm) != 0)
			update_map(&tm);

		if ((te = match_tuple(&tm, input, MAXSUB, regmatch, fflag)) == NULL)
			write(STDOUT_FILENO, NO_MATCH, strlen(NO_MATCH));
		else
		{
			expand_replace(input, te->replace, output, MAXSUB, regmatch);
			write(STDOUT_FILENO, CSTR(output), VSTRLEN(output));
		}

		write(STDOUT_FILENO, "\n", 1);

		if (iflag == 1)
			print_map(&tm);

		debug("-------- wait on select() --------\n");
	}

	return 0;
}

void
drop_privs(char *name)
{
	struct passwd	*user;

	debug("* drop_privs('%s')\n", name);

	if ((user = getpwnam(name)) == NULL)
		fault(1, "user '%s' does not exist\n", name);

	if (setuid(user->pw_uid) != 0)
		fault(2, "failed to setuid(%s): %s\n", name, strerror(errno));
}

