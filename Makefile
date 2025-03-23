HEADERS = linkedlist.h
OBJECTS = fen2svg.o linkedlist.o

all: fen2svg

%.o: %.c $(HEADERS)
	gcc -g -c $< -o $@

fen2svg: $(OBJECTS)
	gcc -g $(OBJECTS) -o $@

clean:
	-rm -f *.o
	-rm -f fen2svg testlist unsortedlinkedlist
