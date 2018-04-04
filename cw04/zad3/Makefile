MAIN = zad3.c
MAINO = zad3.o
MAINEXE = zad3

all: $(MAINO) $(MAINEXE)

clean: 
	rm -r $(MAINO) $(MAINEXE)

$(MAINO): $(MAIN)
	gcc -c $(MAIN)

$(MAINEXE): $(MAINO)
	gcc $(MAINO) -o $(MAINEXE)
