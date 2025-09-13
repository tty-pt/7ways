.SUFFIXES: .o .c

TTYLIBS := qmap qsys
NPMLIBS := ${TTYLIBS:%=@tty-pt/%}
NPMFLAGS := ${NPMLIBS:%=-L%/lib}

BE := gl
LIB-gl := -lX11 -lGL

LDFLAGS := ${NPMFLAGS}
LDLIBS := -lpng -lqsys -lqmap ${LIB-${BE}}

.c.o:
	${CC} -o $@ -c $<

rpg: src/main.c src/be.o src/${BE}.o src/img.o src/png.o
	${CC} -o $@ $^ ${LDFLAGS} ${LDLIBS}
