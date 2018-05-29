MAIN = main.o
MAINEXE = main
CCFLAGS = -lpthread

all: $(MAIN) $(MAINEXE)

clean:
	rm -r $(MAIN) $(MAINEXE)

$(MAIN): main.c
	gcc -c main.c

$(MAINEXE): $(MAIN)
	gcc $(MAIN) -o $(MAINEXE) $(CCFLAGS)
