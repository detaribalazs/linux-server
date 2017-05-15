# Makefile

release_flags := -Wall -c -O3 -g0
debug_flags := -Wall -c -ggdb2 -DDEBUG
objs := main.o config.o
debug_objs := server_debug.o config_debug.o
prgs := server_debug server

all: $(prgs)

.PHONY: all

clean: 
	rm ./*.o $(prgs)

main.o: main.c config.h
	gcc $(release_flags) -L./config.h main.c

server_debug.o: main.c config.h
	gcc $(debug_flags) -o server_debug.o -L./config.h main.c

config.o: config.c config.h
	gcc $(release_flags) -L./config.h config.c

config_debug.o: config.c config.h
	gcc $(debug_flags) -o config_debug.o -L./config.h config.c

server: $(objs)
	gcc $(objs) -o server

server_debug: $(debug_objs)
	gcc $(debug_objs) -o server_debug