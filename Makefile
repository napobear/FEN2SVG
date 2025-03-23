HEADERS = linkedlist.h
OBJECTS = fen2svg.o linkedlist.o

default: fen2svg

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

fen2svg: $(OBJECTS)
	gcc $(OBJECTS) -o $@

testlist: testlist.o unsortedlinkedlist.o
	gcc testlist.c unsortedlinkedlist.c -o testlist

clean:
	-rm -f *.o
	-rm -f fen2svg testlist
