STATICLIB = libchararraylib.a
SHAREDLIB = sharedlibchararraylib.so

LIB = chararraylib.o

all: $(LIB) $(STATICLIB) $(SHAREDLIB)

clean:
	rm -r $(STATICLIB) $(SHAREDLIB) $(LIB)

$(LIB): chararraylib.c
	gcc -fPIC -c chararraylib.c

$(STATICLIB): $(LIB)
	ar rcs $(STATICLIB) $(LIB)

$(SHAREDLIB): $(LIB)
	gcc -shared -o $(SHAREDLIB) $(LIB) -lc


