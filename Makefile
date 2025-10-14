.SUFFIXES: .o .c

TTYLIBS := geo qmap qdb qsys
prefix := /usr/local
# NPMLIBS := ${TTYLIBS:%=@tty-pt/%}
# NPMFLAGS := ${NPMLIBS:%=-L%/lib}

BE := gl
# LIB-gl := -lX11 -lGL
LIB-gl := -lglfw -lGL
INPUT-fb := dev
# INPUT-gl := x
INPUT-gl := dev

LIB-tmx := -ltmx -lz -lxml2

# LDFLAGS := ${NPMFLAGS}
LDFLAGS := ${prefix:%=-L%/lib}
LDLIBS := -lm -lpng ${TTYLIBS:%=-l%} ${LIB-${BE}}

obj-y := game time be ${BE}
obj-y += img png
obj-y += input input-${INPUT-${BE}}
obj-y += tile char
obj-y += view map
obj-y += font dialog

obj-y += shader
# OTHER := tmxc

CFLAGS := -g -Wall -Wpedantic
LDFLAGS += ${prefix:%=-I%/include}

PREFIX ?= /usr/local

.c.o:
	${CC} -o $@ ${CFLAGS} -c $<

all: rpg ${OTHER}

rpg: src/main.c ${obj-y:%=src/%.o}
	${CC} -o $@ $^ ${CFLAGS} ${LDFLAGS} ${LDLIBS}

tmxc: src/tmxc.c src/map.o src/img.o src/png.o
	${CC} -o $@ $^ ${CFLAGS} ${LDFLAGS} ${LIB-tmx} \
		${LDLIBS}

resources := $(DESTDIR)${PREFIX}/share/fb-rpg/resources
map := $(DESTDIR)${PREFIX}/share/fb-rpg/map

install:
	install -d $(DESTDIR)${PREFIX}/bin
	install -m 755 rpg $(DESTDIR)${PREFIX}/bin/fb-rpg
	install -d $(DESTDIR)${PREFIX}/share
	install -d $(DESTDIR)${PREFIX}/share/fb-rpg
	install -d ${resources}
	install -m 644  resources/tiles.png ${resources}
	install -m 644  resources/lamb.png ${resources}
	install -m 644  resources/rooster.png ${resources}
	install -m 644  resources/ui.png ${resources}
	install -m 644  resources/font.png ${resources}
	install -d ${map}
	install -m 644  map/col0.png ${map}
	install -m 644  map/map0.png ${map}
	install -m 644  map/map1.png ${map}
	install -m 644  map/map2.png ${map}
	install -m 644  map/map3.png ${map}
	install -m 644  map/info.txt ${map}

.PHONY: all install
