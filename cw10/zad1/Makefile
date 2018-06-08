CLIENT = client
CLIENTO = client.o
SERVER = server
SERVERO = server.o
CCFLAGS = -lpthread

all: $(SERVERO) $(SERVER) $(CLIENTO) $(CLIENT)

clean:
	rm -r $(SERVERO) $(SERVER) $(CLIENTO) $(CLIENT)

$(SERVERO): server.c
	gcc -c server.c

$(SERVER): $(SERVERO)
	gcc $(SERVERO) -o $(SERVER) $(CCFLAGS)

$(CLIENTO): client.c
	gcc -c client.c

$(CLIENT): $(CLIENTO)
	gcc $(CLIENTO) -o $(CLIENT)
