# Makefile for UD CISC user-level thread library

CC = gcc
CFLAGS = -g

LIBOBJS = t_lib.o 

TSTOBJS = test00.o test01.o test01x.o test03.o test06.o test07.o mytest.o philosophers.o 

# specify the executable 

EXECS = test00 test01 test01x test03 test06 test07 mytest philosophers

# specify the source files

LIBSRCS = t_lib.c

TSTSRCS = test00.c test01.c test01x.c test03.c test06.c test07.c mytest.c philosophers.c

# ar creates the static thread library

t_lib.a: ${LIBOBJS} Makefile
	ar rcs t_lib.a ${LIBOBJS}

# here, we specify how each file should be compiled, what
# files they depend on, etc.

t_lib.o: t_lib.c t_lib.h Makefile
	${CC} ${CFLAGS} -c t_lib.c

test00.o: test00.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test00.c

test01.o: test01.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test01.c

test01x.o: test01x.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test01x.c

test03.o: test03.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test03.c

mytest.o: mytest.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c mytest.c

philosophers.o: philosophers.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c philosophers.c

test06.o: test06.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test06.c

test07.o: test07.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test07.c

test00: test00.o t_lib.a Makefile
	${CC} ${CFLAGS} test00.o t_lib.a -o test00

test01: test01.o t_lib.a Makefile
	${CC} ${CFLAGS} test01.o t_lib.a -o test01

test01x: test01x.o t_lib.a Makefile
	${CC} ${CFLAGS} test01x.o t_lib.a -o test01x

test03: test03.o t_lib.a Makefile
	${CC} ${CFLAGS} test03.o t_lib.a -o test03

test06: test06.o t_lib.a Makefile
	${CC} ${CFLAGS} test06.o t_lib.a -o test06
test07: test07.o t_lib.a Makefile
	${CC} ${CFLAGS} test07.o t_lib.a -o test07

mytest: mytest.o t_lib.a Makefile
	${CC} ${CFLAGS} mytest.o t_lib.a -o mytest

philosophers: philosophers.o t_lib.a Makefile
	${CC} ${CFLAGS} philosophers.o t_lib.a -o philosophers

clean:
	rm -f t_lib.a ${EXECS} ${LIBOBJS} ${TSTOBJS} 
