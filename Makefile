# build an executable named pa1.x from pa1.c
all: pa1.c
	gcc -g -Wall -o pa1.x pa1.c -lpthread

clean:
	$(RM) pa1.x
