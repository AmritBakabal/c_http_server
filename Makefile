bin/main: obj/main.o obj/die.o obj/stringbuffer.o
	gcc -g -o bin/main obj/main.o obj/die.o obj/stringbuffer.o

obj/main.o: src/main.c
	gcc -g -o obj/main.o -D_BSD_SOURCE -c src/main.c

obj/die.o: lib/die/die.c
	gcc -g -o obj/die.o -c lib/die/die.c
	
obj/stringbuffer.o: lib/stringbuffer/stringbuffer.c
	gcc -g -o obj/stringbuffer.o -c lib/stringbuffer/stringbuffer.c

run: bin/main
	./bin/main

debug: bin/main
	gdb ./bin/main