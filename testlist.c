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

#include "unsortedlinkedlist.h"

#define NULL 0

int main() {

    UnsortedList myList;
    myList.First=NULL;
    myList.Last=NULL;

    addToList(&myList, "apple");
    addToList(&myList, "banana");
    addToList(&myList, "cherry");

    modifyItemValue(myList.Last, "clementine");

    displayList(myList);
    
    freeList(&myList);
	
    return 0;
}

