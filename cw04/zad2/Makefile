MAIN = zad2.c
MAINO = zad2.o
MAINEXE = zad2

all: $(MAINO) $(MAINEXE)

clean: 
	rm -r $(MAINO) $(MAINEXE)

$(MAINO): $(MAIN)
	gcc -c $(MAIN)

$(MAINEXE): $(MAINO)
	gcc $(MAINO) -o $(MAINEXE)
