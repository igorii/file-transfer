all:
	gcc -o client client.c -Wall -Werror
	gcc -o server server.c -Wall -Werror

clean:
	rm -f client
	rm -f server
