MAIN = main.o
MAINEXE = main
CCFLAGS = -lpthread -lm

all: $(MAIN) $(MAINEXE)

clean:
	rm -r $(MAIN) $(MAINEXE) result.ascii.pgm

$(MAIN): main.c
	gcc -c main.c

$(MAINEXE): $(MAIN)
	gcc $(MAIN) -o $(MAINEXE) $(CCFLAGS)
