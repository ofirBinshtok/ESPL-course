all: task1 task4

task1: task1.c
	gcc -m32 -g -Wall -o task1 task1.c 

task4: task4.c
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -o task4 task4.c 

.PHONY:clean

clean:
	rm -f *.o task1 task4