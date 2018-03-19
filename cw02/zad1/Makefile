MAIN = main.o
MAINEXE = main
FILE2 = filecp.txt
FILE1 = file.txt
FILE3 = filecplib.txt

all: $(MAIN) $(MAINEXE)

clean:
	rm -r $(MAIN) $(MAINEXE) $(FILE1) $(FILE2) $(FILE3)

$(MAIN): main.c
	gcc -c main.c

$(MAINEXE): $(MAIN)
	gcc $(MAIN) -o $(MAINEXE)

4generate:
	./$(MAINEXE) generate file.txt 4000 4
512generate:
	./$(MAINEXE) generate file.txt 4000 512
4096generate:
	./$(MAINEXE) generate file.txt 2000 4096
8192generate:
	./$(MAINEXE) generate file.txt 2000 8192

4make:
	./$(MAINEXE) copy $(FILE1) $(FILE2) 4000 4 sys
	./$(MAINEXE) copy $(FILE1) $(FILE3) 4000 4 lib
	./$(MAINEXE) sort $(FILE1) 4000 4 sys
	./$(MAINEXE) sort $(FILE2) 4000 4 lib
512make:
	./$(MAINEXE) copy $(FILE1) $(FILE2) 4000 512 sys
	./$(MAINEXE) copy $(FILE1) $(FILE3) 4000 512 lib
	./$(MAINEXE) sort $(FILE1) 4000 512 sys
	./$(MAINEXE) sort $(FILE2) 4000 512 lib
4096make:
	./$(MAINEXE) copy $(FILE1) $(FILE2) 2000 4096 sys
	./$(MAINEXE) copy $(FILE1) $(FILE3) 2000 4096 lib
	./$(MAINEXE) sort $(FILE1) 2000 4096 sys
	./$(MAINEXE) sort $(FILE2) 2000 4096 lib
8192make:
	./$(MAINEXE) copy $(FILE1) $(FILE2) 2000 8192 sys
	./$(MAINEXE) copy $(FILE1) $(FILE3) 2000 8192 lib
	./$(MAINEXE) sort $(FILE1) 2000 8192 sys
	./$(MAINEXE) sort $(FILE2) 2000 8192 lib
