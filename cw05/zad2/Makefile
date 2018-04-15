MASTER = master
MASTERC = master.c
MASTERO = master.o
SLAVE = slave
SLAVEC = slave.c
SLAVEO = slave.o


all: $(MASTERO) $(MASTER) $(SLAVEO) $(SLAVE)

clean:
	rm -r $(MASTERO) $(MASTER) $(SLAVEO) $(SLAVE)

$(MASTERO): $(MASTERC)
	gcc -c $(MASTERC)

$(MASTER): $(MASTERO)
	gcc $(MASTERO) -o $(MASTER)

$(SLAVEO): $(SLAVEC)
	gcc -c $(SLAVEC)

$(SLAVE): $(SLAVEO)
	gcc $(SLAVEO) -o $(SLAVE)
