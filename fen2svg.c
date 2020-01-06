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
FEN2SVG converts chess FEN strings (see PGN standard) to a SVG diagram.
*/

/* Compile source with
      gcc fen2svg.c unsortedlinkedlist.c -o fen2svg
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>              /* POSIX command line arguments parsing */
#include "unsortedlinkedlist.h"  /* Own work */

#define BUFFER_SIZE 1024         /* Holds strings of variable length. */
#define FILE_NAME_MAX_SIZE 1024
#define SVG_TEMPLATE "template.svg"
#define FEN_EXCERPT_LENGTH 75    /* FEN part really useful:
                                    64 fillable squares + 7 row separators +
                                    1 blank space + side to move + '\0'.
                                    Must absolutely be greater than zero. */

/* The SVG template must ABSOLUTELY respect following conventions: */
#define SQUARE_WIDTH 72
#define SQUARE_HEIGHT 72
#define BORDER_THICKNESS 1                   /* Around the chessboard */
#define HORIZONTAL_COORDINATES_HEIGHT 48     /* Horizontal coordinates */
#define VERTICAL_COORDINATES_WIDTH 48        /* Vertical coordinates */
#define MOVE_INDICATOR_WIDTH 72

typedef int bool;
#define true 1
#define false 0

int computeWholeDrawingWidth(bool bCoordinates, bool bBorder,
   bool bMoveIndicator) {
      
   int nDrawingWidth=0;

   if (bCoordinates) {
      nDrawingWidth+=VERTICAL_COORDINATES_WIDTH;
   }
   if (bBorder) {
      nDrawingWidth+=BORDER_THICKNESS;
   }
   nDrawingWidth+=8*SQUARE_WIDTH;
   if (bBorder) {
      nDrawingWidth+=BORDER_THICKNESS;
   }
   if (bMoveIndicator) {
      nDrawingWidth+=MOVE_INDICATOR_WIDTH;
   }

   return nDrawingWidth;
}

int computeWholeDrawingHeight(bool bCoordinates, bool bBorder) {
   
   int nDrawingHeight=0;

   if (bBorder) {
      nDrawingHeight+=BORDER_THICKNESS;
   }
   nDrawingHeight+=8*SQUARE_HEIGHT;
   if (bBorder) {
      nDrawingHeight+=BORDER_THICKNESS;
   }
   if (bCoordinates) {
      nDrawingHeight+=HORIZONTAL_COORDINATES_HEIGHT;
   }

   return nDrawingHeight;
}

//TODO: examiner si je dois renommer en fillboardwith pieces

/* Fill empty board with pieces according FEN string received in input.
 * Doing so, convert each FEN character to its corresponding SVG item. */
bool fillChessboard(char* sFEN, UnsortedList* lstBoardFilling, 
   bool bBorder, bool bCoordinates, bool bMoveIndicator) {

   /* nSquareCount ranges from 0 to 63.
    * File = nSquareCount%8;
    * Rank = nSquareCount/8; (floor() seems better)
    */
/* Une conversion float -> int ne te garantit pas l'exactitude, d'ailleurs la Norme dit que la conversion peut hésiter entre la valaur supérieure et la valeur inférieure. */

   /* INITIALIZE. */
   char sBuffer[BUFFER_SIZE]; //TODO: initiliaser à '\0'?
   char sFENPiece[]="BbKkNnPpQqRr";
   char* asSVGPiece[]={
      "whitebishop", "blackbishop", "whiteking", "blackking", "whiteknight",
      "blackknight", "whitepawn", "blackpawn", "whitequeen", "blackqueen",
      "whiterook", "blackrook"};
   int nTranslateX=0;
   int nTranslateY=0;


   if (bCoordinates) {
      nTranslateX+=VERTICAL_COORDINATES_WIDTH; /* Shift board to the right. */
   }
   if (bBorder) {
      nTranslateX+=BORDER_THICKNESS;
      nTranslateY+=BORDER_THICKNESS;
   }

   /* PARSE FEN. */
   char cCurrentChar;
   int nPos=0;          /* Item currently read in the FEN string. */
   int nSquareCount=0;  /* Range from 0 to 63. */
   while (sFEN[nPos]!='\0' && sFEN[nPos]!=' ' && nSquareCount<64) {
      cCurrentChar=sFEN[nPos];

      /* When a digit is found, jumps as many square as its value. */
      if (cCurrentChar>'0' && cCurrentChar<'9') {
         nSquareCount+=(cCurrentChar-'0');
      }
      else {
         /* Replace piece character (if found) by its SVG string. */
         char* pFound=strchr(sFENPiece, cCurrentChar);
         if (pFound) {
            snprintf(sBuffer,
               BUFFER_SIZE,
               "   <use xlink:href=\"#%s\" x=\"%d\" y=\"%d\" />",
               asSVGPiece[pFound-sFENPiece],
               SQUARE_WIDTH*(nSquareCount%8)+nTranslateX,
               SQUARE_HEIGHT*(nSquareCount/8)+nTranslateY
            );
            addToList(lstBoardFilling, sBuffer);
            nSquareCount++;
         }
         else if (cCurrentChar=='/') {
            /* Use of nSquareCount/8 allows to simply ignore this char. */
         }
         else {
            /* UNALLOWED CHARACTER. */
            printf("Unexpected character (%c) in piece placement of "
               "FEN string.", sFEN[nPos]);
            return false;
         }
      }
      nPos++;
   }

   /* SET UP MOVE INDICATOR */
   if (bMoveIndicator) {
      /* Reach the first blank space. */
      while (sFEN[nPos]!='\0' && sFEN[nPos]!=' ') {
         nPos++;
      };
      /* Reach the first char which is not a blank space. */
      while (sFEN[nPos]!='\0' && sFEN[nPos]==' ') {
         nPos++;
      };
      /* White or black to play? */
      bool bWhiteToPlay=true;
      if (sFEN[nPos]=='b') {
         bWhiteToPlay=false;
      } 
      /* Setting move indicator. */
      nTranslateX=0;
      nTranslateY=0;
      if (bCoordinates) {
         nTranslateX+=VERTICAL_COORDINATES_WIDTH; /* Shift board to the right. */
      }
      if (bBorder) {
         nTranslateX+=BORDER_THICKNESS;
         nTranslateX+=BORDER_THICKNESS;
         nTranslateY+=BORDER_THICKNESS;
      }
      if (bWhiteToPlay) {
         snprintf(
            sBuffer,
            BUFFER_SIZE,
            "   <use xlink:href=\"#moveindicator\" fill=\"white\" x=\"%d\" "
               "y=\"%d\" />",
            SQUARE_WIDTH*8+nTranslateX,
            SQUARE_HEIGHT*7+nTranslateY);
      }
      else {
         snprintf(
            sBuffer,
            BUFFER_SIZE,
            "   <use xlink:href=\"#moveindicator\" fill=\"black\" x=\"%d\" "
               "y=\"%d\" />",
            SQUARE_WIDTH*8+nTranslateX,
            SQUARE_HEIGHT*7+nTranslateY);
      }
      addToList(lstBoardFilling, sBuffer);
   }

   return true;
}

/* Read FEN file, adding each FEN string to a linked list. */
bool readFENFile(char* sFileName, UnsortedList* lstFENString) {
   

   FILE* fInputFile = fopen(sFileName, "rt") ;
   if (fInputFile==NULL) {
      printf("Error: cannot open input file (%s).\n", sFileName);
      return false;
   }

   /* BROWSE FILE LINE BY LINE */
   char sFENExcerpt[FEN_EXCERPT_LENGTH];
   char sFileLine[BUFFER_SIZE];
   int nLine=1;   /* For more readable error messages. */   //TODO: remove this code: the is no error message.
   while (fgets(sFileLine, BUFFER_SIZE, fInputFile)) {
      strncpy(sFENExcerpt, sFileLine, FEN_EXCERPT_LENGTH);
      sFENExcerpt[FEN_EXCERPT_LENGTH]='\0';
      addToList(lstFENString, sFENExcerpt);
      nLine++;
   }
   
   /* CLOSE FILE */
   fclose(fInputFile);

   return true;
}

/* Read SVG definitions, adding each line to a linked list. */
bool readSVGTemplate(char* sFileName, UnsortedList* lstEmptyBoard) {   //TODO: vérifier si la ligne lue > BUFFER_SIZE

   FILE* fInputFile;
   char sBuffer[BUFFER_SIZE];

   /* OPEN FILE */
   fInputFile = fopen(sFileName, "rt") ;
   if (fInputFile==NULL) {
      printf("Error: cannot open input file (%s).", sFileName);
      return false;
   }

   /* BROWSE FILE LINE BY LINE */
   while (fgets(sBuffer, BUFFER_SIZE, fInputFile)) { // fgets() + sscanf() > fscanf() !?
      /* Remove trailing '\n'. */
      if (sBuffer) {
         int nBufferLastChar=strlen(sBuffer)-1;
         if (sBuffer[nBufferLastChar]=='\n') {
            sBuffer[nBufferLastChar]='\0';
         }
      }
      /* Add line to list. */
      addToList(lstEmptyBoard, sBuffer);
   }

   /* CLOSE FILE */
   fclose(fInputFile);

   return true;
}

/* Append SVG length and width to opening tag ("<svg>") and suppress
 * closing tag (will be recreated upon SVG completion).
 * This step could be done during template loading. However the goal
 * here is to keep loading separated for reusability and maintenance. */
bool adaptSVGTemplate(UnsortedList lstSVGTemplate,
   bool bBorder, bool bCoordinates, bool bMoveIndicator) {

   char sBuffer[BUFFER_SIZE];        /* Holds length variations. */

   /* Point to start of the list. */
   ListItem* itmCurrentItem=lstSVGTemplate.First;

   /* Append width and length to starting tag. */
   if (itmCurrentItem && itmCurrentItem->Value) {
      if (strncmp("<svg", itmCurrentItem->Value, 4)==0) {
         snprintf(
            sBuffer,
            BUFFER_SIZE,
            "<svg width=\"%d\" height=\"%d\" version=\"1.1\"\n",
            computeWholeDrawingWidth(bCoordinates,bBorder, bMoveIndicator),
            computeWholeDrawingHeight(bCoordinates, bBorder)
         );
         modifyItemValue(itmCurrentItem, sBuffer);
      }
      else {
         printf("Template first line is not '<svg' <> '%s': halting.\n",
         itmCurrentItem->Value);
         return false;
      }
   }
   else {
      printf("Template first line not found : halting.\n"); //TODO: mettre un exit_failure
      return false;
   }

   /* Reach the last item. */
   while(itmCurrentItem->Next) { /* itmCurrentItem always exist. */
      itmCurrentItem=itmCurrentItem->Next;
   }

   /* Delete closing tag. */
   if (itmCurrentItem && itmCurrentItem->Value) {
      if (strncmp("</svg>", itmCurrentItem->Value,
         strlen("</svg>"))==0) {
         modifyItemValue(itmCurrentItem, "\n");
      }
      else {
         printf("Template last line is not '</svg>' <> '%s': halting.\n",
            itmCurrentItem->Value);
         return false;
      }
   }
   else {
      printf("Template last line not found or empty: halting.");
      return false;
   }
      
   return true;
}

/* Features (border, coordinates, move indicator, ...) can be added to the 
 * chessboard where the pieces will be placed. However, every position output 
 * uses the same board. It can be therefore once for all. */
bool generateEmptyChessboard(UnsortedList lstEmptyBoard,
   bool bBorder, bool bCoordinates, bool bMoveIndicator) {
      
   /* INITIALIZE ITEM LOCATION ON THE SVG DRAWING. */
   /* Location of an item is defined by (nX+nTranslateX, nY+nTranslateY). */
   int nX=0;            /* Location on a stripped board. */
   int nY=0;
   int nTranslateX=0;   /* Allows to insert item(s) before current one. */
   int nTranslateY=0;

   /* SET UP LIGHT AND DARK SQUARES. */
   if (bCoordinates) {
      nTranslateX+=VERTICAL_COORDINATES_WIDTH;  /* Shift board to the right. */
   }
   if (bBorder) {
      nTranslateX+=BORDER_THICKNESS;
      nTranslateY+=BORDER_THICKNESS;
   }
   char sBuffer[BUFFER_SIZE];          /* Holds length variations. */
   bool bLightSquare=true;
   for (nY=0; nY<8; nY++) {            /* Eight rows. */
      for (nX=0; nX<8; nX++) {         /* Eight columns. */
         if (bLightSquare) {
            snprintf(sBuffer,
               BUFFER_SIZE,
               "   <use xlink:href=\"#lightsquare\" x=\"%d\" y=\"%d\" />",
               nX*SQUARE_WIDTH+nTranslateX,
               nY*SQUARE_HEIGHT+nTranslateY);
            addToList(&lstEmptyBoard, sBuffer);
         }
         else {
            snprintf(sBuffer,
               BUFFER_SIZE,
               "   <use xlink:href=\"#darksquare\" x=\"%d\" y=\"%d\" />",
               nX*SQUARE_WIDTH+nTranslateX,
               nY*SQUARE_HEIGHT+nTranslateY);
            addToList(&lstEmptyBoard, sBuffer);
         }
         bLightSquare=!bLightSquare;   /* Switch square color. */
      }
      bLightSquare=!bLightSquare;
   }

   /* SET UP BORDERS. */
   if (bBorder) {
      /* Initialize. */
      nX=0;
      nTranslateX=0;
      if (bCoordinates) {
         nTranslateX+=VERTICAL_COORDINATES_WIDTH; /* Shift board to the right. */
      }
      /* Generate XML line. */
      snprintf(
         sBuffer,
         BUFFER_SIZE,
         "   <use xlink:href=\"#borders\" x=\"%d\" y=\"0\" />",
         nX+nTranslateX
      );
      /* Append line to XML list for output file. */
      addToList(&lstEmptyBoard, sBuffer);
   }

   /* SET UP COORDINATES. */
   if (bCoordinates) {
      /* Vertical coordinates (from '8' to '1'). */
      nY=0;
      nTranslateY=BORDER_THICKNESS;
      for(char cCoordinate='8'; cCoordinate>'0'; cCoordinate-='1'-'0') {
         /* Generate SVG line. */
         snprintf(
            sBuffer,
            BUFFER_SIZE,
            "   <use xlink:href=\"#coordinate%c\" x=\"0\" y=\"%d\" />",
            cCoordinate,
            nY+nTranslateY
         );
         /* Append line to list for output file. */
         addToList(&lstEmptyBoard, sBuffer);
         nY+=SQUARE_HEIGHT;
      }
      /* Horizontal coordinates (from 'a' to 'h') */
      nX=0;
      nY=8*SQUARE_HEIGHT;
      nTranslateX=VERTICAL_COORDINATES_WIDTH;
      nTranslateX+=BORDER_THICKNESS;
      nTranslateY=BORDER_THICKNESS;
      nTranslateY+=BORDER_THICKNESS;
      for(char cCoordinate='a'; cCoordinate<'i'; cCoordinate+='b'-'a') {
         /* Generate SVG line. */
         snprintf(
            sBuffer,
            BUFFER_SIZE,
            "   <use xlink:href=\"#coordinate%c\" x=\"%d\" y=\"%d\" />",
            cCoordinate,
            nX+nTranslateX,
            nY+nTranslateY
         );
         /* Append line to list for output file. */
         addToList(&lstEmptyBoard, sBuffer);
         nX+=SQUARE_WIDTH;
      }
   }
}

/* Merge empty board & positions into SVG files. */
bool writeDiagrams(UnsortedList lstEmptyBoard, UnsortedList lstFENString,
   bool bBorder, bool bCoordinates, bool bMoveIndicator) {

   char sOutputFile[FILE_NAME_MAX_SIZE];
   FILE* fOutputFile;
   UnsortedList lstBoardFilling;
   lstBoardFilling.First=NULL;      /* useless here but more safe */
   lstBoardFilling.Last=NULL;       /* useless here but more safe */
   ListItem* itmBrowseFENStrings=NULL;
   ListItem* itmBrowseEmptyBoard=NULL;
   ListItem* itmBrowseBoardFilling=NULL;
   int nDiagramNumber=1;


   itmBrowseFENStrings=lstFENString.First;
   while(itmBrowseFENStrings) {
      if (itmBrowseFENStrings->Value) {
         
         /* Analyze current FEN. */
         lstBoardFilling.First=NULL;
         lstBoardFilling.Last=NULL;
         fillChessboard(itmBrowseFENStrings->Value, &lstBoardFilling,
            bBorder, bCoordinates, bMoveIndicator);

         /* Create file */
         snprintf(sOutputFile, FILE_NAME_MAX_SIZE, "dia%05d.svg",
            nDiagramNumber);

         /* Open file */
         fOutputFile = fopen(sOutputFile, "w");
         if (fOutputFile == NULL) {
            printf("Error: cannot open output file (%s).", sOutputFile);
            return false;
         }

         /* Write empty board */
         itmBrowseEmptyBoard=lstEmptyBoard.First;
         while(itmBrowseEmptyBoard) {
            if (itmBrowseEmptyBoard->Value) {
               fprintf(fOutputFile, "%s\n", itmBrowseEmptyBoard->Value);
            }
            itmBrowseEmptyBoard=itmBrowseEmptyBoard->Next;
         }

         /* Write board filling */
         itmBrowseBoardFilling=lstBoardFilling.First;
         while(itmBrowseBoardFilling) {
            if (itmBrowseBoardFilling->Value) {
               fprintf(fOutputFile, "%s\n", itmBrowseBoardFilling->Value);
            }
            itmBrowseBoardFilling=itmBrowseBoardFilling->Next;
         }
         fprintf(fOutputFile, "</svg>\n");

         /* Close file */
         fclose(fOutputFile);
         
         /* Get next item */
         freeList(&lstBoardFilling);
         itmBrowseFENStrings=itmBrowseFENStrings->Next;
         nDiagramNumber++;
      }
   }

   return true;
}

int main(int argc, char *argv[]) {

   enum InputMode {UNKNOWN_MODE, FILE_MODE, STRING_MODE}; /* Do we receive the
      FEN strings directly or are they stored in a file? */

   enum InputMode enuInputMode = UNKNOWN_MODE;
   bool bBorder=false;
   bool bCoordinates=false;
   bool bMoveIndicator=false;

   /* 1 - PARSE COMMAND LINE ARGUMENTS */
   if (argc==1) {
      // No argument found.
      fprintf(stderr, "%s: missing operand\n", argv[0]);
      fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
      exit(EXIT_FAILURE);
   }
   int c;
   while ( (c = getopt(argc, argv, "hbcmfs")) != -1) {
      switch (c) {
         case 'h':
            /* Display help */
            printf("FEN2SVG v0.1 (June 2 2019): convert chess positions ("
               "described in FEN \n");
            printf("strings) into diagrams (written in SVG).\n");
            printf("This program is free software: you can redistribute "
               "and/or modify it.\n");
            printf("FEN2SVG comes with ABSOLUTELY NO WARRANTY, to the "
               "extent permitted by \n");
            printf("applicable law.\n");
            printf("Written by Michaël I. F. George.\n");
            printf("License GPLv3+: GNU GPL version 3 or later \n");
            printf("<http://gnu.org/licenses/gpl.html>\n");
            printf("\n");
            fprintf(stderr, "Usage: %s [-bcmfs] file(s) or string(s)\n",
               argv[0]);
            printf("   -b\tborders\n");
            printf("   -c\texternal coordinates\n");
            printf("   -m\tmove indicator\n");
            printf("   -f\tfile mode (default):\n");
            printf("   \tFEN positions are contained in a file\n");
            printf("   -s\tstring mode:\n");
            printf("   \tFEN posititions are passed directly in the command "
               "line\n");
            return EXIT_SUCCESS;
            break;
         case 'b':
            bBorder = true;
            break;
         case 'c':
            bCoordinates=true;
            break;
         case 'm':
            bMoveIndicator=true;
            break;
         case 'f':
            /* Store input file name */
            if (enuInputMode!=UNKNOWN_MODE && enuInputMode!=FILE_MODE) {
               fprintf(stderr, "%s: only one input mode (string or file) "
                  "can be selected at a time\n", argv[0]);
               fprintf(stderr, "Try '%s -h' for more information.\n",
                  argv[0]);
               exit(EXIT_FAILURE);
            }
            else {
               enuInputMode = FILE_MODE;
            }
            
            //sInputFile = optarg;
            break;
         case 's':
            /* Store FEN string */
            if (enuInputMode!=UNKNOWN_MODE && enuInputMode!=STRING_MODE) {
               fprintf(stderr, "%s: only one input mode (string or file) "
                  "can be selected at a time\n", argv[0]);
               fprintf(stderr, "Try '%s -h' for more information.\n",
                  argv[0]);
               exit(EXIT_FAILURE);
            }
            else {
               enuInputMode = STRING_MODE;
            }
            //sInputFEN = optarg;
            break;
         case '?':
            break;
         default:
            fprintf (stderr, "?? getopt returned character code 0%o ??\n", c);
      }
   }
    /* Set file as default input mode. */
   if (enuInputMode==UNKNOWN_MODE) {
      enuInputMode=FILE_MODE;
   }
   /* Non-optional arguments (options are preceeded by a '-')
      Here FEN file(s) or string(s) are expected. */ 
   UnsortedList lstArgument;    /* File names or FEN strings */
   lstArgument.First=NULL;
   lstArgument.Last=NULL;
   if (optind>=argc) {
      fprintf(stderr, "%s: no file or FEN string to process\n", argv[0]);
      fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
      exit(EXIT_FAILURE);
   }
   else {
      while (optind<argc) {
         addToList(&lstArgument, argv[optind++]);
      }
   }

   /* 2 - READ INPUT FEN STRINGS. */
   UnsortedList lstFENString;
   lstFENString.First=NULL;
   lstFENString.Last=NULL;
   if (enuInputMode=FILE_MODE) { /* Get FEN strings from file(s). */
      ListItem* lstBrowse=lstArgument.First; /* Avoid to modify lstArgument. */
      while(lstBrowse) {
         if (lstBrowse->Value) {
            readFENFile(lstBrowse->Value, &lstFENString);
         }
         lstBrowse=lstBrowse->Next;
      /* lstBrowse is null now. */
      }
   }
   else {                        /* FEN(s) directly passed as argument. */
      ListItem* lstBrowse=lstArgument.First; /* Avoid to modify lstArgument. */
      while(lstBrowse) {
         if (lstBrowse->Value) {
            addToList(&lstFENString, lstBrowse->Value);
         }
         lstBrowse=lstBrowse->Next;
      }
      /* lstBrowse is null now. */
   }
   freeList(&lstArgument);

   /* 3 - GENERATE EMPTY CHESSBOARD (same board is used for every position) */
   UnsortedList lstEmptyBoard;
   lstEmptyBoard.First=NULL;
   lstEmptyBoard.Last=NULL;
   /* 3.1 - Read template */
   if (!readSVGTemplate(SVG_TEMPLATE, &lstEmptyBoard)) {
      freeList(&lstEmptyBoard);
      return EXIT_FAILURE;
   }
   /* 3.2 - Adjust template. */
   if (!adaptSVGTemplate(lstEmptyBoard, bBorder, bCoordinates,
      bMoveIndicator)) {
      freeList(&lstEmptyBoard);
      return EXIT_FAILURE;
   }
   /* 3.3 - Generate empty board. */
   if (!generateEmptyChessboard(lstEmptyBoard,
      bBorder, bCoordinates, bMoveIndicator)) {   
      freeList(&lstEmptyBoard);
      return EXIT_FAILURE;
   }

   /* 4 - GENERATE AND WRITE DOWN SVG DIAGRAMS. */
   writeDiagrams(lstEmptyBoard, lstFENString, 
      bBorder, bCoordinates, bMoveIndicator);

   /* 5 - FREE MEMORY. */
   freeList(&lstFENString);
   freeList(&lstEmptyBoard);
   
   /* 6 - RETURN "EVERYTHING WENT WELL". */
   return EXIT_SUCCESS;

   //TODO: tester if (BUFFER_SIZE>snprintf())
   //TODO: supprimer le sscanf: comme cela on peut soumettre également des .epd et comme les champs autres que piece placement, on s'en fiche
   //TODO: dénombrer lignes du fichier FEN pour déterminer sur combien de position coder le numéro des diagrammes.
   //TODO: examiner les exit(EXIT_FAILURE) vs return(EXIT_FAILURE)
   //TODO: examiner les fprintf(stderr, vs printf
   //TODO: mettre toutes les erreurs en fprintf(stderr
   //TODO: virer tous les commentaires du style "//"
   //TODO: choisir les lettres pour les options
   //TODO: enlever le 1234567890123456789... de l'aide
   //TODO: régler bogue du fichier qui ne contient qu'un '\n'.

   //DONE: la const FILE_NAME_MAX_SIZE n'accepte que 128 caractères pour un nom de fichier:
   //       1) voir si on peut la remplacer par BUFFER_SIZE
   //       2) voir si on teste le cas où nom de fichier > 128
   //TODO: avoir un count dans le type (c.f. pointeur de fin) dans la liste, cela permettra de compléter convenablement à gauche le numéro du diagramme.
   //DONE: renommer en BoardFiller(), BoardFilling(), BoardStuffer(), BoardStuffing().
   //DONE: chaque nouveau UnsortedList doit être suivi de .First=NULL et .Last=NULL
   //DONE: créer un type UnsortedList, plutôt que "struct Item**" dans le .h
   //DONE: optimiser l'ajout avec un pointeur dernier
   //DONE: enlever le itmFirstItem de la fonction addToList_New() dans unsortedlinkedlist.c
   //TODO: accented character accepted or not
   //TODO: to choose a format for comments (C old standard or C++ format)
   //TODO: remplacer les strcpy par strncpy



}
