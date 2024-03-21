all: loader

task0: task0.c
	gcc -m32 -g -Wall -o task0 task0.c 

task1: task1.c
	gcc -m32 -g -Wall -o task1 task1.c 

loader: loader.o startup.o start.o
	ld -o loader loader.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2 

loader.o: loader.c
	gcc -m32 -c loader.c -o loader.o

start.o: start.s
	nasm -f elf32 start.s -o start.o

startup.o: startup.s
	nasm -f elf32 startup.s -o startup.o

.PHONY:clean

clean:
	rm -f *.o task0 task1 loader

	
