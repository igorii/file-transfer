all: helper.o
	gcc -o client client.c helper.o -Wall -Werror
	gcc -o server server.c helper.o -Wall -Werror

helper.o: helper.c helper.h
	gcc -c helper.c

clean:
	rm -f client
	rm -f server
	rm -f *.o
