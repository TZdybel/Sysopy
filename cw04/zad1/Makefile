MAIN = main.c
MAINO = main.o
MAINEXE = main

all: $(MAINO) $(MAINEXE)

clean: 
	rm -r $(MAINO) $(MAINEXE)

$(MAINO): $(MAIN)
	gcc -c $(MAIN)

$(MAINEXE): $(MAINO)
	gcc $(MAINO) -o $(MAINEXE)
