#include <stdio.h>
#include <unistd.h>

#include "debug.h"
#include "vstring.h"

int
main(int argc, char *argv[])
{
	vstring	*foo,
			*bar;

	foo = vstrnew("");

	vstrread(STDIN_FILENO, foo, 1024);

	printf("foo = '%s'\n", CSTR(foo));

	bar = vstresc(CSTR(foo));
	vstrdel(&foo);

	printf("bar = '%s'\n", CSTR(bar));

	return 0;
}
