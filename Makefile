.SUFFIXES: .o .c

LDLIBS := -lpng -lqsys

.c.o:
	${CC} -o $@ -c $<

rpg: src/fb.o src/png.o
	${CC} -o $@ src/main.c $^ ${LDLIBS}

src/fb.o: src/fb.c
src/png.o: src/png.c
