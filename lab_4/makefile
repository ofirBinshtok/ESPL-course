all: hexeditplus task4

hexeditplus: hexeditplus.c
	gcc -m32 -g -Wall -o hexeditplus hexeditplus.c 

task4: task4.c
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -o task4 task4.c 

.PHONY:clean

clean:
	rm -f *.o hexeditplus task4