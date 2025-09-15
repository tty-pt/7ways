.SUFFIXES: .o .c

TTYLIBS := qmap qsys
NPMLIBS := ${TTYLIBS:%=@tty-pt/%}
NPMFLAGS := ${NPMLIBS:%=-L%/lib}

BE := gl
LIB-gl := -lX11 -lGL
INPUT-fb := dev
INPUT-gl := x

obj-y := be ${BE} img png input
obj-y += input-${INPUT-${BE}}

LDFLAGS := ${NPMFLAGS}
LDLIBS := -lpng -lqsys -lqmap ${LIB-${BE}}

.c.o:
	${CC} -o $@ -c $<

rpg: src/main.c ${obj-y:%=src/%.o}
	${CC} -o $@ $^ ${LDFLAGS} ${LDLIBS}
