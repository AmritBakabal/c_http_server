main: main.o die.o
	gcc -o main main.o die.o

main.o: main.c
	gcc -c main.c

die.o: die.c
	gcc -c die.c

run: main
	./main
