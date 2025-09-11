.SUFFIXES: .o .c

.c.o:
	${CC} -o $@ -c $<

rpg: src/fb.o
	${CC} -o $@ src/main.c $<

rpg: src/main.c include/draw.h
src/fb.o: src/fb.c include/draw.h
