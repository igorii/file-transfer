all: dirs helper.o menu.o net_commands.o
	gcc -o bin/client client/client.c obj/menu.o obj/net_commands.o obj/helper.o -Wall -Werror -lm -Ilib
	gcc -o bin/server server/server.c obj/helper.o -Wall -Werror -lm -Ilib

dirs:
	mkdir -p obj
	mkdir -p bin

net_commands.o: client/net_commands.c client/net_commands.h
	gcc -c client/net_commands.c -o obj/net_commands.o -Ilib

menu.o: client/menu.c client/menu.h
	gcc -c client/menu.c -o obj/menu.o

helper.o: lib/helper.c lib/helper.h
	gcc -c lib/helper.c -o obj/helper.o

clean:
	rm -rf obj
	rm -rf bin
