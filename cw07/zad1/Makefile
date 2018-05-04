all: GOLIBRODAO GOLIBRODA KLIENTO KLIENT

clean:
	rm -r golibroda.o golibroda klient.o klient

GOLIBRODAO: golibroda.c
	gcc -c golibroda.c

GOLIBRODA: golibroda.o
	gcc golibroda.o -o golibroda

KLIENTO: klient.c
	gcc -c klient.c

KLIENT: klient.o
	gcc klient.o -o klient
