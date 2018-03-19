STATICLIB = libchararraylib.a
SHAREDLIB = libchararraylib.so.1
MAIN = main.o
STATICMAIN = statmain
SHAREDMAIN = sharmain
LIB = chararraylib.o

all: $(LIB) $(STATICLIB) $(SHAREDLIB) $(MAIN) $(STATICMAIN) $(SHAREDMAIN) run

clean:
	rm -r $(LIB) $(STATICLIB) $(SHAREDLIB) $(MAIN) $(STATICMAIN) $(SHAREDMAIN)

$(LIB): chararraylib.c
	gcc -fPIC -c chararraylib.c

$(STATICLIB): $(LIB)
	ar rcs $(STATICLIB) $(LIB)

$(SHAREDLIB): $(LIB)
	gcc -shared -o $(SHAREDLIB) $(LIB) -lc

$(MAIN): main.c
	gcc -c main.c

$(STATICMAIN): $(STATICLIB) $(MAIN)
	gcc $(MAIN) $(STATICLIB) -o $(STATICMAIN)

$(SHAREDMAIN): main.c $(SHAREDLIB)
	gcc main.c -lchararraylib -L ./ -o $(SHAREDMAIN)
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tzdybel/Pulpit/sysopy/zad3a

run:
	./$(STATICMAIN)
	./$(SHAREDMAIN)



