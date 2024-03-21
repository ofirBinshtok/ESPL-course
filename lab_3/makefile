all: task2

util.o: util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

main.o: main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

task2: util.o main.o
	nasm -f elf32 start.s -o start.o
	ld -m elf_i386 main.o util.o start.o -o task2

.PHONY: clean

clean: 
	rm -f *.o task2