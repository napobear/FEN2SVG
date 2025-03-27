HEADERS = linkedlist.h
SOURCES = fen2svg.c linkedlist.c
PROGRAM = fen2svg
OBJECTS = fen2svg.o linkedlist.o
FILES   = template.svg Makefile
DOCS    = COPYING README.md README.pdf fen2svg.odt example.png example.fen  example.svg lucas.fen
TOOLS   = unsortedlinkedlist.c testlist.c unsortedlinkedlist.h
VERSION = 0.1

all: $(PROGRAM) dist

%.o: %.c $(HEADERS)
	gcc -g -c $< -o $@

$(PROGRAM): $(OBJECTS)
	gcc -g $(OBJECTS) -o $@

dist: $(PROGRAM)
	mkdir -p $(PROGRAM)-$(VERSION)
	cp $(HEADERS) $(SOURCES) $(FILES) $(DOCS) $(TOOLS) $(PROGRAM)-$(VERSION)/
	tar cvzf $(PROGRAM)-$(VERSION).tgz $(PROGRAM)-$(VERSION)

clean:
	-rm -f *.o
	-rm -f $(PROGRAM)
	-rm -f $(PROGRAM)-$(VERSION).tgz
	-rm -f $(PROGRAM)-$(VERSION)/*
	-if [ -d $(PROGRAM)-$(VERSION) ]; then rmdir $(PROGRAM)-$(VERSION); fi
