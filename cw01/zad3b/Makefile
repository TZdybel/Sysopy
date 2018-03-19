STATICLIB = libchararraylib.a
SHAREDLIB = libchararraylib.so.1
MAIN = main.o
STATICMAIN = statmain
SHAREDMAIN = sharmain
LIB = chararraylib.o

all: $(LIB) $(STATICLIB) $(SHAREDLIB) $(MAIN) $(STATICMAIN) $(SHAREDMAIN)

clean:
	rm -r $(LIB) $(STATICLIB) $(SHAREDLIB) $(MAIN) $(STATICMAIN) $(SHAREDMAIN)

$(LIB): chararraylib.c
	gcc -fPIC -O3 -c chararraylib.c

$(STATICLIB): $(LIB)
	ar rcs $(STATICLIB) $(LIB)

$(SHAREDLIB): $(LIB)
	gcc -O3 -shared -o $(SHAREDLIB) $(LIB) -lc

$(MAIN): main.c
	gcc -O3 -c main.c

$(STATICMAIN): $(STATICLIB) $(MAIN)
	gcc -O3 $(MAIN) $(STATICLIB) -o $(STATICMAIN)

$(SHAREDMAIN): main.c $(SHAREDLIB)
	gcc -O3 main.c -lchararraylib -L ./ -o $(SHAREDMAIN)
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tzdybel/Pulpit/sysopy/zad3a




