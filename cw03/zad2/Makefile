MAIN = main.o
MAINEXE = main


all: $(MAIN) $(MAINEXE)

clean:
	rm -r $(MAIN) $(MAINEXE)

cleantest:
	rm -r testowykatalog plik.txt

$(MAIN): main.c
	gcc -c main.c

$(MAINEXE): $(MAIN) $(LIB)
	gcc $(MAIN) $(LIB) -o $(MAINEXE)
