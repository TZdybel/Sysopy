all: SERVERO SERVER CLIENTO CLIENT

clean:
	rm -r server.o server client.o client

SERVERO: server.c
	gcc -c server.c

SERVER: server.o
	gcc server.o -o server

CLIENTO: client.c
	gcc -c client.c

CLIENT: client.o
	gcc client.o -o client
