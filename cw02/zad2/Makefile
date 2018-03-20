MAIN = dirsearch.o
MAINEXE = dirsearch

all: $(MAIN) $(MAINEXE)

clean:
	rm -r $(MAIN) $(MAINEXE)

$(MAIN): dirsearch.c
	gcc -c dirsearch.c

$(MAINEXE): $(MAIN)
	gcc $(MAIN) -o $(MAINEXE)
	
