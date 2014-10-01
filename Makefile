all: dirs helper.o
	gcc -o bin/client client/client.c obj/helper.o -Wall -Werror -lm -Ilib
	gcc -o bin/server server/server.c obj/helper.o -Wall -Werror -lm -Ilib

dirs:
	mkdir -p obj
	mkdir -p bin

helper.o: lib/helper.c lib/helper.h
	gcc -c lib/helper.c -o obj/helper.o

clean:
	rm -rf obj
	rm -rf bin
