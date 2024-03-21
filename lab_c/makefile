all: myshell looper mypipeline

mypipeline: mypipeline.c
	gcc -m32 -g -Wall -o mypipeline mypipeline.c

myshell: myshell.c LineParser.c LineParser.h
	gcc -m32 -g -Wall -o myshell myshell.c LineParser.c

looper: looper.c
	gcc -m32 -g -Wall -o looper looper.c

.PHONY: clean
clean:
	rm -f mypipeline myshell looper