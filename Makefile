BE := gl

INPUT-fb := dev
INPUT-gl := glfw

-include ../mk/glfw.mk

LDLIBS := -lm -lgeo -lxxhash -lqmap -lqsys -lpng
LDLIBS += ${LDLIBS-${BE}}

obj-y := game time be ${BE}
obj-y += img png
obj-y += input input-${INPUT-${BE}}
obj-y += tile char
obj-y += view map
obj-y += font dialog
obj-y += shader
7ways-obj-y := ${obj-y:%=src/%.o}

tmxc-obj-y := map img png
tmxc-obj-y := ${tmxc-obj-y:%=src/%.o}
LDLIBS-tmxc := -ltmx -lz -lxml2

pngs :=  press seven tiles lamb rooster ui font
resources := ${pngs:%=resources/%.png}
map := col0 map0 map1 map2 map3
map := ${map:%=map/%.png} map/info.txt
share := map.txt ${resources} ${map}
share-dirs := resources map

CFLAGS := -g

-include ../mk/include.mk
