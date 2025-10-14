.SUFFIXES: .o .c

uname := $(shell test "${cross}" = "" && uname || echo ${cross})
uname != test "${cross}" = "" && uname || echo ${cross}
arch := $(shell uname -m)
arch != uname -m

prefix-Darwin-arm64  := /opt/homebrew
prefix-Darwin-x86_64 := /usr/local
prefix-Darwin += ${prefix-Darwin-${arch}}
prefix-Linux := /usr
prefix-OpenBSD := /usr/local /usr/X11R6
prefix-Msys := /mingw64
prefix := ${prefix-${uname}}

cc-Linux := ${CC}
cc-Darwin := ${CC}
cc-OpenBSD := ${CC}
cc-Msys := /usr/bin/x86_64-w64-mingw32-gcc
cc := ${cc-${uname}}

exe-Msys := .exe
exe := ${exe-${uname}}

TTYLIBS := geo xxhash qmap qsys
# NPMLIBS := ${TTYLIBS:%=@tty-pt/%}
# NPMFLAGS := ${NPMLIBS:%=-L%/lib}

BE := gl
# LIB-gl := -lX11 -lGL
GL-Linux := -lGL
GL-OpenBSD := -lGL -lGLU -lX11
LIB-glfw-Msys := -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32
LIB-glfw-Linux := -lglfw
LIB-glfw-OpenBSD := -lglfw
LIB-glfw-Darwin := -lglfw
LIB-gl := ${LIB-glfw-${uname}} ${GL-${uname}}

INPUT-fb := dev
# INPUT-gl := x
# INPUT-gl := dev
INPUT-gl := glfw

LIB-tmx := -ltmx -lz -lxml2

# LDFLAGS := ${NPMFLAGS}
#
LDFLAGS-Darwin := -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

LDFLAGS := ${prefix:%=-L%/lib} ${LDFLAGS-${uname}}
LDLIBS := -lm -lpng ${TTYLIBS:%=-l%} ${LIB-${BE}}

obj-y := main game time be ${BE}
obj-y += img png
obj-y += input input-${INPUT-${BE}}
obj-y += tile char
obj-y += view map
obj-y += font dialog

obj-y += shader
# OTHER := tmxc

CFLAGS := -g -Wall -Wpedantic
CFLAGS += ${prefix:%=-I%/include}

PREFIX ?= /usr/local

.c.o:
	${cc} -o $@ ${CFLAGS} -c $<

all: rpg${exe} ${OTHER}

rpg$(exe): src/main.c ${obj-y:%=src/%.o}
	${cc} -o $@ ${obj-y:%=src/%.o} ${CFLAGS} ${LDFLAGS} ${LDLIBS}

tmxc: src/tmxc.c src/map.o src/img.o src/png.o
	${cc} -o $@ src/map.o src/img.o src/png.o ${CFLAGS} ${LDFLAGS} ${LIB-tmx} \
		${LDLIBS}

resources := $(DESTDIR)${PREFIX}/share/fb-rpg/resources
map := $(DESTDIR)${PREFIX}/share/fb-rpg/map

install:
	install -d $(DESTDIR)${PREFIX}/bin
	install -m 755 rpg${exe} $(DESTDIR)${PREFIX}/bin/fb-rpg
	install -d $(DESTDIR)${PREFIX}/share
	install -d $(DESTDIR)${PREFIX}/share/fb-rpg
	install -d ${resources}
	install -m 644  resources/press.png ${resources}
	install -m 644  resources/seven.png ${resources}
	install -m 644  resources/start.png ${resources}
	install -m 644  resources/logo.png ${resources}
	install -m 644  resources/lamb.png ${resources}
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
