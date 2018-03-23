DIRSEARCH = dirSearchSepProc.o
DIRSEAMAIN = dirSearchSepProc
CFILE = dirSearchSepProc.c

all: $(DIRSEARCH) $(DIRSEAMAIN)

clean: 
	rm -r $(DIRSEARCH) $(DIRSEAMAIN)

$(DIRSEARCH): $(CFILE)
	gcc -c $(CFILE)

$(DIRSEAMAIN): $(DIRSEARCH)
	gcc $(DIRSEARCH) -o $(DIRSEAMAIN)
