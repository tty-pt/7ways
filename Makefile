.SUFFIXES: .o .c

TTYLIBS := geo qdb qmap qsys
NPMLIBS := ${TTYLIBS:%=@tty-pt/%}
NPMFLAGS := ${NPMLIBS:%=-L%/lib}

BE := gl
LIB-gl := -lX11 -lGL
INPUT-fb := dev
INPUT-gl := x

LDFLAGS := ${NPMFLAGS}
LDLIBS := -lm -lpng ${TTYLIBS:%=-l%} ${LIB-${BE}}

obj-y := game time be ${BE}
obj-y += img png
obj-y += input input-${INPUT-${BE}}
obj-y += tile char view

obj-y += shader

CFLAGS := -g

.c.o:
	${CC} -o $@ ${CFLAGS} -c $<

rpg: src/main.c ${obj-y:%=src/%.o}
	${CC} -o $@ $^ ${CFLAGS} ${LDFLAGS} ${LDLIBS}
