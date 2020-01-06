/*
Copyright 2019 Michaël I. F. George

This file is part of FEN2SVG.

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

/*
As the name suggests, this code is the header file for unsortedlinkedlist.c, 
a helper file for FEN2SVG. 
*/

struct LinkedListItem {
   char* Value;
   struct LinkedListItem* Next;
};
typedef struct LinkedListItem ListItem;

struct UnsortedLinkedList {
   ListItem* First;        /* Browsing the list begins here. */
   ListItem* Last;         /* Adding items occurs faster. */
};
typedef struct UnsortedLinkedList UnsortedList;

void addToList(UnsortedList* lstList, char* sValue);
void modifyItemValue(ListItem* itmCurrent, char* sValue);
void displayList(UnsortedList lstList);
void freeList(UnsortedList* lstList);
