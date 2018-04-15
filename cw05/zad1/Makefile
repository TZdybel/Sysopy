MAIN = main.o
MAINEXE = main


all: $(MAIN) $(MAINEXE)

clean:
	rm -r $(MAIN) $(MAINEXE)

cleantest:
	rm -r plik.txt

$(MAIN): main.c
	gcc -c main.c

$(MAINEXE): $(MAIN)
	gcc $(MAIN) -o $(MAINEXE)
