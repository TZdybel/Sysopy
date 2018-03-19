MAIN = main.o
LIB = chararraylib.o
MAINEXE = main


all: $(MAIN) $(LIB) $(MAINEXE)

clean:
	rm -r $(MAIN) $(LIB) $(MAINEXE)

$(MAIN): main.c
	gcc -c main.c

$(LIB): chararraylib.c
	gcc -c chararraylib.c

$(MAINEXE): $(MAIN) $(LIB)
	gcc $(MAIN) $(LIB) -o $(MAINEXE)
