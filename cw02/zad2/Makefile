MAIN = dirsearch.o
MAINEXE = dirsearch
NFTW = nftw.o
NFTWEXE = nftw

all: $(MAIN) $(MAINEXE) $(NFTW) $(NFTWEXE)

clean:
	rm -r $(MAIN) $(MAINEXE) $(NFTW) $(NFTWEXE)

$(MAIN): dirsearch.c
	gcc -c dirsearch.c

$(MAINEXE): $(MAIN)
	gcc $(MAIN) -o $(MAINEXE)
	
$(NFTW): nftw.c
	gcc -c nftw.c
	
$(NFTWEXE): $(NFTW)
	gcc $(NFTW) -o $(NFTWEXE)
