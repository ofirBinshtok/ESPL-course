all: task1

util.o: util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

task1: util.o
	nasm -f elf32 start.s -o start.o
	ld -m elf_i386 util.o start.o -o encoder

.PHONY: clean

clean: 
	rm -f *.o encoder