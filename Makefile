INSTALL_DIR = /app/apache/active/conf/common

CC=		/opt/SUNWspro/bin/cc
CFLAGS=	-fast
#CFLAGS+=-DDEBUG
#CFLAGS+=-g

OBJS=	main.o debug.o porthelp.o tuple.o vstring.o
TOBJS=	test.o debug.o porthelp.o vstring.o

rereplace: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

test: $(TOBJS)
	$(CC) $(CFLAGS) $(TOBJS) -o $@

main.o:			debug.h main.h porthelp.h tuple.h vstring.h

debug.o:		debug.h

porthelp.o:		debug.h porthelp.h vstring.h

tuple.o:		debug.h main.h tuple.h vstring.h

vstring.o:		debug.h vstring.h

test.o:			debug.h porthelp.h vstring.h

clean:
	rm -f $(OBJS) rereplace
	rm -f $(TOBJS) test

install:    rereplace
	cp rereplace $(INSTALL_DIR)/rewrite_map

