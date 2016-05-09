# Makefile for UD CISC user-level thread library

CC = gcc
CFLAGS = -g

LIBOBJS = t_lib.o 

TSTOBJS = test00.o test01.o test01x.o test01a.o test02.o test04.o test07.o mytest.o 

# specify the executable 

EXECS = test00 test01 test01x test01a test02 test04 test07 mytest

# specify the source files

LIBSRCS = t_lib.c

TSTSRCS = test00.c test01.c test01x.c test01a.c test02.c test04.c test07.c mytest.c

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

test01a.o: test01a.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test01a.c

test02.o: test02.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test02.c

test04.o: test04.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test04.c

mytest.o: mytest.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c mytest.c

test07.o: test07.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test07.c

test00: test00.o t_lib.a Makefile
	${CC} ${CFLAGS} test00.o t_lib.a -o test00

test01: test01.o t_lib.a Makefile
	${CC} ${CFLAGS} test01.o t_lib.a -o test01

test01x: test01x.o t_lib.a Makefile
	${CC} ${CFLAGS} test01x.o t_lib.a -o test01x

test01a: test01a.o t_lib.a Makefile
	${CC} ${CFLAGS} test01a.o t_lib.a -o test01a

test02: test02.o t_lib.a Makefile
	${CC} ${CFLAGS} test02.o t_lib.a -o test02

test04: test04.o t_lib.a Makefile
	${CC} ${CFLAGS} test04.o t_lib.a -o test04

test07: test07.o t_lib.a Makefile
	${CC} ${CFLAGS} test07.o t_lib.a -o test07

mytest: mytest.o t_lib.a Makefile
	${CC} ${CFLAGS} mytest.o t_lib.a -o mytest

clean:
	rm -f t_lib.a ${EXECS} ${LIBOBJS} ${TSTOBJS} 
