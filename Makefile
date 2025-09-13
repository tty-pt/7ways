.SUFFIXES: .o .c

TTYLIBS := qmap qsys
NPMLIBS := ${TTYLIBS:%=@tty-pt/%}
NPMFLAGS := ${NPMLIBS:%=-L%/lib}

LDFLAGS := ${NPMFLAGS}
LDLIBS := -lpng -lqsys -lqmap

.c.o:
	${CC} -o $@ -c $<

rpg: src/main.c src/fb.o src/img.o src/png.o
	${CC} -o $@ $^ ${LDFLAGS} ${LDLIBS}
