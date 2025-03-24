HEADERS = linkedlist.h
OBJECTS = fen2svg.o linkedlist.o

default: fen2svg

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

fen2svg: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f program
