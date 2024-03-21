all: myshell mypipe looper

myshell: myshell.o LineParser.o
	gcc -m32 -g -Wall -o myshell myshell.o LineParser.o

mypipe: mypipe.c 
	gcc -m32 -g -Wall -o mypipe mypipe.c 

LineParser.o: LineParser.c
	gcc -c -m32 -g -Wall -c -o LineParser.o LineParser.c

myshell.o: myshell.c
	gcc -c -m32 -g -Wall -c -o myshell.o myshell.c

looper: looper.c
	gcc -m32 -g -Wall -o looper looper.c

.PHONY: clean
clean:
	rm -f myshell mypipe looper