# Makefile

release_flags := -Wall -c -O3 -g0
debug_flags := -Wall -c -ggdb2
objs := server.o config.o
debug_objs := server_debug.o config_debug.o
prgs := main_debug main

all: $(prgs)

.PHONY: all

clean: 
	rm ./*.o $(prgs)

server.o: server.c config.h
	gcc $(release_flags) -L./config.h server.c

server_debug.o: server.c config.h
	gcc $(debug_flags) -o server_debug.o -L./config.h server.c

config.o: config.c config.h
	gcc $(release_flags) -L./config.h config.c

config_debug.o: config.c config.h
	gcc $(debug_flags) -o config_debug.o -L./config.h config.c

main: $(objs)
	gcc $(objs) -o main

main_debug: $(debug_objs)
	gcc $(debug_objs) -o main_debug