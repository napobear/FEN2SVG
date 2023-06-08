/**
 * Copyright 2019-2023 Michaël I. F. George
 * 
 * This file is part of FEN2SVG.
 * 
 * FEN2SVG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * FEN2SVG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 **/

/**
 * As the name suggests, this code offers other programs a very rudimentary
 * unsorted linked list, on a FIFO modus operandi. It is meant to be used
 * with by FEN2SVG.
 **/


#include<stdlib.h>  /* malloc(), free(), exit() */
#include<stdio.h>   /* printf() */
#include<string.h>  /* strlen(), strcpy() */
#include "linkedlist.h"


LinkedList* createEmptyList(void) {
    
    LinkedList* lstList = NULL;
    lstList = (LinkedList*) malloc(1 * sizeof(LinkedList));
    if (!lstList) {
        printf("Unsuccessful malloc() in createEmptyList(): halting.\n");
        exit(EXIT_FAILURE);
    }

    (*lstList).First = NULL;
    (*lstList).Last = NULL;

    return lstList;
}


/* Parameter itmFirstItem is not really needed. */
void appendToList(LinkedList* lstList, char* sValue) {
    ListItem* itmNew;

    /* CREATE ITEM */
    itmNew = (ListItem*) malloc(1 * sizeof(ListItem));
    if (itmNew == NULL) {
        exit(EXIT_FAILURE);
    }
    /* Copy string value. */
    itmNew->Value = (char*) malloc((strlen(sValue)+1)*sizeof(char));
    if (!(itmNew->Value)) {
        exit(EXIT_FAILURE); /* Memory allocation failed. */
    }
    strcpy(itmNew->Value, sValue);
    /* Set next item. */
    itmNew->Next = NULL;

    /* PLACE THE NEW ITEM AT THE END OF THE LINKED LIST */
    if (!(*lstList).First) { /* <=> Is the list empty? */
        (*lstList).First = itmNew;
    }
    else {
        /* Make the last item of the list point to the new item. */
        (*lstList).Last->Next = itmNew;
    }
    /* Define new item as last item. */
    (*lstList).Last = itmNew;
}

void modifyItemValue(ListItem* itmCurrent, char* sValue) {

    /* Did we received a NULL pointer? */
    if (!itmCurrent) {
        exit(EXIT_FAILURE);
    }

    /* Free previously stored string. */
    if (itmCurrent->Value) {
        free(itmCurrent->Value);
    }

    /* Store string value into item. */
    itmCurrent->Value = (char*) malloc((strlen(sValue)+1)*sizeof(char));
    if (!(itmCurrent->Value)) {
        exit(EXIT_FAILURE); /* Memory allocation failed. */
    }
    strcpy(itmCurrent->Value, sValue);
}

void displayList(LinkedList lstList) {
    
    ListItem* itmCurrent = lstList.First;

    printf("----\n");
    if (!itmCurrent) {
        printf("list is empty");
    }
    
    while(itmCurrent) {
        printf("%s\n", itmCurrent->Value);
        itmCurrent = itmCurrent->Next;
    }
    printf("----\n");
}


LinkedList* copyList(LinkedList lstSource) {

    LinkedList* lstDestination = createEmptyList();

    ListItem* itmSource = lstSource.First;
    while(itmSource) {
        appendToList(lstDestination, itmSource->Value);
        itmSource = itmSource->Next;
    }
    
    return lstDestination;
}


void freeList(LinkedList** lstList) {
    ListItem* itmCurrent;
    ListItem* itmStoreNext;

    itmCurrent = (**lstList).First;
    while (itmCurrent) {
        itmStoreNext = itmCurrent->Next;    /* Store next address, before it is unreachable. */
        if (itmCurrent->Value) {            /* Delete stored string. */
            free(itmCurrent->Value);
            itmCurrent->Value = NULL;
        }
        if (itmCurrent) {                   /* Delete item itself. */
            free(itmCurrent);
            itmCurrent = NULL;
        }
        itmCurrent = itmStoreNext;
    }

    (**lstList).First = NULL;               /* Pointer to first item of the list */
    (**lstList).Last = NULL;                /* Pointer to last item of the list. */

    free(*lstList);                         /* Free list "entrance". */
}
