#ifndef _REREPLACE_H_
#define _REREPLACE_H_

#define NO_MATCH	"NULL"

#define MAXINPUT	65536
#define INPUTCHUNK	1023

#define MAXSUB		10

#define CHECK_FREQ	10

#ifndef SETUID
#define SETUID		"apache"
#endif

/* drop privileges, if running as root */
void drop_privs(char *name);

#endif /* _REREPLACE_H_ */
