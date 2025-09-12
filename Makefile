.SUFFIXES: .o .c

LDFLAGS := -L./node_modules/@tty-pt/qsys/lib
LDFLAGS += -L./node_modules/@tty-pt/qmap/lib
LDLIBS := -lpng -lqsys -lqmap

.c.o:
	${CC} -o $@ -c $<

rpg: src/fb.o src/img.o src/png.o src/tm.o
	${CC} -o $@ src/main.c $^ ${LDFLAGS} ${LDLIBS}
