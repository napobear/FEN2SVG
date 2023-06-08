/*
Copyright 2019 Michaël I. F. George

This file is part of FEN2SVG. Its purpose is to allow testing of
unsortedlinkelist.c.

FEN2SVG is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FEN2SVG is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/


/* Compile source with
 *      gcc testlist.c unsortedlinkedlist.c -o testlist
 * Check for memory leaks with
 *      gcc -g -o0 testlist.c unsortedlinkedlist.c -o testlist
 *      valgrind -v --leak-check=full ./testlist
 * Validate code with
 *      splint unsortedlinkedlist.c testlist.c
 * Debug code with GDB
 *      gdb ./testlist
*/




#include "unsortedlinkedlist.h"

#define NULL 0

int main() {

    UnsortedList* myList = createEmptyList();

    appendToList(myList, "apple");
    appendToList(myList, "banana");
    appendToList(myList, "cherry");

    modifyItemValue((*myList).Last, "clementine");

    displayList(*myList);
    
    freeList(&myList);
	
    return 0;
}

