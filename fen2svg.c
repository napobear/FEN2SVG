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
 * FEN2SVG converts chess FEN strings (see PGN standard) to a SVG diagram.
 *
 * The SVG diagrams produced are made of 3 sets of SVG lines:
 *   - definitions (e.g.: how to draw a black knight, a dark square, move indicator, ...),
 *   - empty board (i.e.: place a light square on (x, y), uses definitions),
 *   - chess pieces (i.e.: place a black knight on (x, y), uses definitions).
 **/

/**
 * Compile source with
 *      gcc fen2svg.c linkedlist.c -o fen2svg
 *
 * Check for memory leaks with
 *      gcc -g -o0 linkedlist.c fen2svg.c -o fen2svg
 *      valgrind -v --leak-check=full ./fen2svg
 *
 * Validate code with
 *      splint linkedlist.c fen2svg.c
 *
 * Debug code with GDB
 *      gdb --args ./fen2svg -bmrp objectif_2000.tsv
 **/

#include "linkedlist.h" /* Own work */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* POSIX command line arguments parsing (getopt) */

#define BUFFER_SIZE 1024 /* Holds strings of variable length. */
#define FILE_NAME_MAX_SIZE 1024
#define SVG_TEMPLATE "template.svg"
#define FEN_EXCERPT_LENGTH 75 /* Only the 75st chars of FEN are really useful: \
                                 64 fillable squares + 7 row separators +      \
                                 1 blank space + side to move + '\0'.          \
                                 Must absolutely be greater than zero. */
#define WHITE_ON_BOTTOM true
#define BLACK_ON_BOTTOM false

/* The SVG template must ABSOLUTELY respect following conventions: */
#define SQUARE_WIDTH 72
#define SQUARE_HEIGHT 72
#define BORDER_THICKNESS 2 /* Around the chessboard */
#define HORIZONTAL_COORDINATES_HEIGHT 48 /* Horizontal coordinates */
#define VERTICAL_COORDINATES_WIDTH 48 /* Vertical coordinates */
#define MOVE_INDICATOR_WIDTH 72

#define NUMBERED_FILE_NAME_FORMAT "dia%05d.svg"

/**
 * Width of the board drawing vary on the presence of coordinates, the width of the border, ...
 **/
int computeWholeDrawingWidth(bool bCoordinates, bool bBorder, bool bMoveIndicator)
{

    int nDrawingWidth = 0;

    if (bCoordinates) {
        nDrawingWidth += VERTICAL_COORDINATES_WIDTH;
    }
    if (bBorder) {
        nDrawingWidth += BORDER_THICKNESS;
    }
    nDrawingWidth += 8 * SQUARE_WIDTH;
    if (bBorder) {
        nDrawingWidth += BORDER_THICKNESS;
    }
    if (bMoveIndicator) {
        nDrawingWidth += MOVE_INDICATOR_WIDTH;
    }

    return nDrawingWidth;
}

/**
 * Height of the board drawing vary on the presence of coordinates, the width of the border, ...
 **/
int computeWholeDrawingHeight(bool bCoordinates, bool bBorder)
{

    int nDrawingHeight = 0;

    if (bBorder) {
        nDrawingHeight += BORDER_THICKNESS;
    }
    nDrawingHeight += 8 * SQUARE_HEIGHT;
    if (bBorder) {
        nDrawingHeight += BORDER_THICKNESS;
    }
    if (bCoordinates) {
        nDrawingHeight += HORIZONTAL_COORDINATES_HEIGHT;
    }

    return nDrawingHeight;
}

/**
 * Examine a FEN string to know which side is to play.
 * If side to play is missing, true is returned.
 **/
bool isWhiteToPlay(char* sFEN)
{

    bool bReturnValue = true;
    int nPos = 0;

    /* Reach the first blank space. */
    while (sFEN[nPos] != '\0' && sFEN[nPos] != ' ') {
        nPos++;
    };
    /* Then reach the first char which is not a blank space. */
    while (sFEN[nPos] != '\0' && sFEN[nPos] == ' ') {
        nPos++;
    };
    /* White or black to play? */
    if (sFEN[nPos] != '\0' && sFEN[nPos] == 'b') {
        bReturnValue = false;
    }

    return bReturnValue;
}

/**
 * Generate a file name with a FEN string as input.
 * Doing so, convert each FEN character to its corresponding SVG item.
 * <p>
 * This empty chessboard is intented to act as a template to create a board filled with chess
 * pieces later.
 *
 * @param   sFEN            FEN string representing the chess position
 * @param   lstBoardFilling an unsorted linked list of SVG lines
 * @param   bBorder         frame around the board requested?
 * @param   bCoordinates    coordinates for algebric notation around the board
 * @param   bMoveIndicator  little picture next to the board telling who is to move
 * @param   bRotateBoard    board orientation
 * @return
 * @see     generateEmptyBoard()
 **/
char* generateFENFileName(char* sFEN)
{

    const char* sAdmittedCharacters = "1pP2348RrkK5bBNn6qQ7";

    /* ALLOCATE MEMORY FOR STRING */
    char* sReturnValue = (char*)malloc(strlen(sFEN) * sizeof(char) + 1 + 4);
    if (!sReturnValue) {
        printf("Unsuccessful malloc() in generateFENFileName(): halting.\n");
        exit(EXIT_FAILURE);
    }

    /* PARSE FEN. */
    char cCurrentChar;
    int nInputPos = 0;
    int nOutputPos = 0;
    while ((cCurrentChar = sFEN[nInputPos]) != '\0' && cCurrentChar != ' ') {
        if (strchr(sAdmittedCharacters, cCurrentChar)) {
            sReturnValue[nOutputPos++] = cCurrentChar;
        }
        nInputPos++;
    }

    /* APPEND SIDE TO PLAY ('w' or 'b') */
    /* Reach first non-blank char. */
    while (sFEN[nInputPos] != '\0' && sFEN[nInputPos] == ' ') {
        nInputPos++;
    };
    /* Side to play. */
    if (sFEN[nInputPos] != '\0' && sFEN[nInputPos] == 'b') { /* If missing, 'w' is appended). */
        sReturnValue[nOutputPos++] = 'b';
    } else {
        sReturnValue[nOutputPos++] = 'w';
    }

    /* APPEND FILE EXTENSION */
    sReturnValue[nOutputPos++] = '.';
    sReturnValue[nOutputPos++] = 's';
    sReturnValue[nOutputPos++] = 'v';
    sReturnValue[nOutputPos++] = 'g';
    sReturnValue[nOutputPos++] = '\0';

    /* */
    return sReturnValue;
}

/**
 * Generate a file name of the form "dia00130.svg"
 **/
char* generateNumberedFileName(int nDiagramNumber)
{

    /* ALLOCATE MEMORY FOR STRING */
    char* sReturnValue = (char*)malloc(12 * sizeof(char) + 1); /* 12 = length of "dia%05d.svg". */
    if (!sReturnValue) {
        printf("Unsuccessful malloc() in generateNumberedFileName(): halting.\n");
        exit(EXIT_FAILURE);
    }

    /* */
    if (!snprintf(sReturnValue, 13, NUMBERED_FILE_NAME_FORMAT, nDiagramNumber)) {
        printf("Unsuccessful snprintf() in writeDiagramsToFiles(): halting.\n");
        exit(EXIT_FAILURE);
    }

    /* */
    return sReturnValue;
}

/**
 * Create a list of pieces to add to an empty chessboard.
 * To do that it parse the FEN string received in input.
 * Each character of the string represents a chess piece.
 * Thus, each FEN character is converted to a SVG line (i.e. a drawing of a chess piece).
 *
 * @param   lstPieces       an EMPTY list of SVG lines
 * @param   sFEN            FEN string representing the chess position
 * @param   bBorder         frame around the board requested?
 * @param   bCoordinates    coordinates for algebric notation around the board
 * @param   bMoveIndicator  little picture next to the board telling who is to move
 * @param   bRotateBoard    board orientation
 * @return  lstReturnValue  a filled list of SVG lines (a piece is drawn with SVG several lines)
 * @see     generateEmptyBoard()
 **/
LinkedList* createPieces(char* sFEN, bool bBorder, bool bCoordinates, bool bMoveIndicator,
    bool bRotateBoard)
{

    /* nSquareCount ranges from 0 to 63.
     * File = nSquareCount % 8;
     * Rank = nSquareCount / 8; (floor() seems better)
     */
    /* Une conversion float -> int ne te garantit pas l'exactitude, d'ailleurs la Norme dit que la conversion peut hésiter entre la valaur supérieure et la valeur inférieure. */

    /* INITIALIZE. */
    char sBuffer[BUFFER_SIZE]; // TODO: initiliaser à '\0'?
    char sFENPiece[] = "BbKkNnPpQqRr";
    char* asSVGPiece[] = { "whitebishop", "blackbishop", "whiteking", "blackking", "whiteknight",
        "blackknight", "whitepawn", "blackpawn", "whitequeen", "blackqueen",
        "whiterook", "blackrook" };
    int nTranslateX = 0;
    int nTranslateY = 0;
    bool bWhiteToPlay = true;

    /* */
    LinkedList* lstReturnValue = createEmptyList();

    /* DETERMINE WHICH SIDE IS TO MOVE */
    bWhiteToPlay = isWhiteToPlay(sFEN);

    /* COORDINATES */
    if (bCoordinates) {
        nTranslateX += VERTICAL_COORDINATES_WIDTH; /* Shift board to the right. */
    }

    /* BORDER */
    if (bBorder) {
        nTranslateX += BORDER_THICKNESS;
        nTranslateY += BORDER_THICKNESS;
    }

    /* PARSE FEN. */
    char cCurrentChar;
    int nPos = 0; /* Item currently read in the FEN string. */
    int nSquareCount = 0; /* Range from 0 to 63. */
    while (sFEN[nPos] != '\0' && sFEN[nPos] != ' ' && nSquareCount < 64) {
        cCurrentChar = sFEN[nPos];

        /* When a digit is found, jumps as many square as its value. */
        if (cCurrentChar > '0' && cCurrentChar < '9') {
            nSquareCount += (int)(cCurrentChar - '0');
        } else {
            /* Replace piece character (if found) by its SVG string. */
            char* pFound = strchr(sFENPiece, cCurrentChar);
            if (pFound) {
                if (bWhiteToPlay || !bRotateBoard) {
                    /* White at bottom. */
                    if (!snprintf(sBuffer,
                            BUFFER_SIZE,
                            "    <use xlink:href = \"#%s\" x = \"%d\" y = \"%d\" />",
                            asSVGPiece[pFound - sFENPiece],
                            SQUARE_WIDTH * (nSquareCount % 8) + nTranslateX,
                            SQUARE_HEIGHT * (nSquareCount / 8) + nTranslateY)) {
                        printf("Unsuccessful snprintf() in fillBoard(): halting.\n");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    /* Black at bottom. */
                    if (!snprintf(sBuffer,
                            BUFFER_SIZE,
                            "    <use xlink:href = \"#%s\" x = \"%d\" y = \"%d\" />",
                            asSVGPiece[pFound - sFENPiece],
                            SQUARE_WIDTH * (7 - nSquareCount % 8) + nTranslateX,
                            SQUARE_HEIGHT * (7 - nSquareCount / 8) + nTranslateY)) {
                        printf("Unsuccessful snprintf() in fillBoard(): halting.\n");
                        exit(EXIT_FAILURE);
                    }
                }
                appendToList(lstReturnValue, sBuffer);
                nSquareCount++;
            } else if (cCurrentChar == '/') {
                /* Use of nSquareCount/8 allows to simply ignore this char. */
            } else {
                /* UNALLOWED CHARACTER. */
                printf("\nERROR: unexpected character (%c) in piece placement of FEN string (%s).",
                    sFEN[nPos], sFEN);
                return false;
            }
        }
        nPos++;
    }

    /* SET UP MOVE INDICATOR */
    if (bMoveIndicator) {
        /* Setting move indicator. */
        nTranslateX = 0;
        nTranslateY = 0;
        if (bCoordinates) {
            nTranslateX += VERTICAL_COORDINATES_WIDTH; /* Shift board to the right. */
        }
        if (bBorder) {
            nTranslateX += BORDER_THICKNESS;
            nTranslateX += BORDER_THICKNESS;
            nTranslateY += BORDER_THICKNESS;
        }
        if (bWhiteToPlay) {
            if (!snprintf(
                    sBuffer,
                    BUFFER_SIZE,
                    "    <use xlink:href = \"#moveindicator\" fill = \"white\" x = \"%d\" "
                    "y = \"%d\" />",
                    SQUARE_WIDTH * 8 + nTranslateX,
                    SQUARE_HEIGHT * 7 + nTranslateY)) {
                printf("Unsuccessful snprintf() in fillBoard(): halting.\n");
                exit(EXIT_FAILURE);
            }
        } else {
            if (!snprintf(
                    sBuffer,
                    BUFFER_SIZE,
                    "    <use xlink:href = \"#moveindicator\" fill = \"black\" x = \"%d\" "
                    "y = \"%d\" />",
                    SQUARE_WIDTH * 8 + nTranslateX,
                    SQUARE_HEIGHT * 7 + nTranslateY)) {
                printf("Unsuccessful snprintf() in fillBoard(): halting.\n");
                exit(EXIT_FAILURE);
            }
        }
        appendToList(lstReturnValue, sBuffer);
    }

    return lstReturnValue;
}

/**
 * Transfert FEN positions from file to linked list.
 **/
bool readFENFile(char* sFileName, LinkedList* lstFENString)
{

    FILE* fInputFile = fopen(sFileName, "rt");
    if (fInputFile == NULL) {
        printf("Error: cannot open input file (%s).\n", sFileName);
        return false;
    }

    /* BROWSE FILE LINE BY LINE */
    char sFENExcerpt[FEN_EXCERPT_LENGTH];
    char sFileLine[BUFFER_SIZE];
    while (fgets(sFileLine, BUFFER_SIZE, fInputFile)) {
        strncpy(sFENExcerpt, sFileLine, FEN_EXCERPT_LENGTH); /* Only first chars are useful. */
        sFENExcerpt[FEN_EXCERPT_LENGTH] = '\0';
        appendToList(lstFENString, sFENExcerpt);
    }

    /* CLOSE FILE */
    if (!fclose(fInputFile)) {
        return false;
    }

    return true;
}

/**
 * Reads SVG definitions from file and puts every line in a linked list.
 * <p>
 * The template is a SVG file that contains only definitions ("<defs/>").
 * For a definition item to be visible, it has to be used ("<use/>").
 *
 * @param   sFileName       name of the SVG
 * @return  lstEmptyBoard   an unsorted linked list of SVG lines
 * @see     generateEmptyBoard()
 **/
LinkedList* readTemplate(char* sFileName)
{ // TODO: vérifier si la ligne lue > BUFFER_SIZE

    LinkedList* lstReturnValue = createEmptyList();

    FILE* fInputFile;
    char sBuffer[BUFFER_SIZE];

    /* OPEN FILE */
    fInputFile = fopen(sFileName, "rt");
    if (fInputFile == NULL) {
        printf("Error: cannot open input file (%s).", sFileName);
        return false;
    }

    /* BROWSE FILE LINE BY LINE */
    while (fgets(sBuffer, BUFFER_SIZE, fInputFile)) { // fgets() + sscanf() > fscanf() !?
        /* Remove trailing '\n'. */
        if (sBuffer) {
            int nBufferLastChar = strlen(sBuffer) - 1;
            if (sBuffer[nBufferLastChar] == '\n') {
                sBuffer[nBufferLastChar] = '\0';
            }
        }
        /* Add line to list. */
        appendToList(lstReturnValue, sBuffer);
    }

    /* CLOSE FILE */
    fclose(fInputFile);

    return lstReturnValue;
}

/** Append SVG length and width to opening tag ("<svg>") and
 * suppress closing tag (which will be recreated upon SVG completion).
 * <p>
 * Lengths of the diagram varies with presence of borders, coordinates and move indicator.
 * <p>
 * This step could be done during template loading. However the goal
 * here is to keep loading separated for reusability and maintenance.
 *
 * @param   bBorder         frame around the board
 * @param   bCoordinates    coordinates for algebric notation around the board
 * @param   bMoveIndicator  little picture next to the board telling who is to move
 * @param   lstSVGTemplate  each item of the list is a SVG line
 * @return  lstSVGTemplate
 * @see     createPieces()
 **/
bool addLengthsToTemplate(LinkedList lstSVGTemplate, bool bBorder, bool bCoordinates,
    bool bMoveIndicator)
{

    char sBuffer[BUFFER_SIZE]; /* Holds length variations. */

    /* POINT TO START OF THE LIST. */
    ListItem* itmCurrentItem = lstSVGTemplate.First;

    /* APPEND WIDTH AND LENGTH TO STARTING TAG. */
    if (itmCurrentItem && itmCurrentItem->Value) {
        if (strncmp("<svg", itmCurrentItem->Value, 4) == 0) {
            if (!snprintf(
                    sBuffer,
                    BUFFER_SIZE,
                    "<svg width = \"%d\" height = \"%d\" version = \"1.1\"\n",
                    computeWholeDrawingWidth(bCoordinates, bBorder, bMoveIndicator),
                    computeWholeDrawingHeight(bCoordinates, bBorder))) {
                printf("Unsuccessful snprintf() in addLengthsToTemplate(): halting.\n");
                exit(EXIT_FAILURE);
            }
            modifyItemValue(itmCurrentItem, sBuffer);
        } else {
            printf("Template first line is not '<svg' <> '%s': halting.\n", itmCurrentItem->Value);
            return false;
        }
    } else {
        printf("Template first line not found : halting.\n"); // TODO: mettre un exit_failure
        return false;
    }

    /* REACH THE LAST ITEM. */
    while (itmCurrentItem->Next) { /* itmCurrentItem always exist. */
        itmCurrentItem = itmCurrentItem->Next;
    }

    /* DELETE CLOSING TAG. */
    if (itmCurrentItem && itmCurrentItem->Value) {
        if (strncmp("</svg>", itmCurrentItem->Value,
                strlen("</svg>"))
            == 0) {
            modifyItemValue(itmCurrentItem, "\n");
        } else {
            printf("Template last line is not '</svg>' <> '%s': halting.\n",
                itmCurrentItem->Value);
            return false;
        }
    } else {
        printf("Template last line not found or empty: halting.");
        return false;
    }

    return true;
}

/**
 * Return a list of uses of SVG definitions to represent an empty chess board.
 * The colour of the square may vary, the board can have a border, coordinates, ...
 * <p>
 * This empty chessboard is intented to act as a template to create a board filled with chess
 * pieces later.
 *
 * @param   bBorder         frame around the board requested?
 * @param   bCoordinates    coordinates for algebric notation around the board
 * @param   bMoveIndicator  little picture next to the board telling who is to move
 * @param   bWhiteAtBottom  board orientation
 * @return  lstEmptyBoard   an unsorted linked list of SVG lines
 * @see     fillBoard()
 **/
LinkedList* generateEmptyBoard(bool bBorder, bool bCoordinates, bool bMoveIndicator,
    bool bWhiteAtBottom)
{

    LinkedList* lstEmptyBoard = createEmptyList();

    /* INITIALIZE ITEM LOCATION ON THE SVG DRAWING. */
    /* Location of an item is defined by (nX+nTranslateX, nY+nTranslateY). */
    int nX = 0; /* Location on a stripped board. */
    int nY = 0;
    int nTranslateX = 0; /* Allows to insert item(s) before current one. */
    int nTranslateY = 0;

    /* SET UP LIGHT AND DARK SQUARES. */
    if (bCoordinates) {
        nTranslateX += VERTICAL_COORDINATES_WIDTH; /* Shift board to the right. */
    }
    if (bBorder) {
        nTranslateX += BORDER_THICKNESS;
        nTranslateY += BORDER_THICKNESS;
    }
    char sBuffer[BUFFER_SIZE]; /* Holds length variations. */
    bool bLightSquare = true; /* alternates between dark and light squares*/
    for (nY = 0; nY < 8; nY++) { /* Eight rows. */
        for (nX = 0; nX < 8; nX++) { /* Eight columns. */
            if (bLightSquare) {
                if (!snprintf(sBuffer, BUFFER_SIZE,
                        "    <use xlink:href = \"#lightsquare\" x = \"%d\" y = \"%d\" />",
                        nX * SQUARE_WIDTH + nTranslateX,
                        nY * SQUARE_HEIGHT + nTranslateY)) {
                    printf("Unsuccessful snprintf() in generateEmptyBoard(): halting.\n");
                    exit(EXIT_FAILURE);
                }
                appendToList(lstEmptyBoard, sBuffer);
            } else {
                if (!snprintf(sBuffer,
                        BUFFER_SIZE,
                        "    <use xlink:href = \"#darksquare\" x = \"%d\" y = \"%d\" />",
                        nX * SQUARE_WIDTH + nTranslateX,
                        nY * SQUARE_HEIGHT + nTranslateY)) {
                    printf("Unsuccessful snprintf() in generateEmptyBoard(): halting.\n");
                    exit(EXIT_FAILURE);
                }
                appendToList(lstEmptyBoard, sBuffer);
            }
            bLightSquare = !bLightSquare; /* Switch square color. */
        }
        bLightSquare = !bLightSquare;
    }

    /* SET UP BORDERS. */
    if (bBorder) {
        /* Initialize. */
        nX = 0;
        nTranslateX = 0;
        if (bCoordinates) {
            nTranslateX += VERTICAL_COORDINATES_WIDTH; /* Shift board to the right. */
        }
        /* Generate XML line. */
        if (!snprintf(
                sBuffer,
                BUFFER_SIZE,
                "    <use xlink:href = \"#borders\" x = \"%d\" y = \"0\" />",
                nX + nTranslateX)) {
            printf("Unsuccessful snprintf() in generateEmptyBoard(): halting.\n");
            exit(EXIT_FAILURE);
        }
        /* Append line to XML list for output file. */
        appendToList(lstEmptyBoard, sBuffer);
    }

    /* SET UP COORDINATES. */
    if (bCoordinates) {
        /* Vertical coordinates (from '8' to '1'). */
        nY = 0;
        nTranslateY = BORDER_THICKNESS;
        if (bWhiteAtBottom) {
            /* White on bottom. */
            for (char cCoordinate = '8'; cCoordinate > '0'; cCoordinate -= '1' - '0') {
                /* Generate SVG line. */
                if (!snprintf(sBuffer, BUFFER_SIZE,
                        "    <use xlink:href = \"#coordinate%c\" x = \"0\" y = \"%d\" />",
                        cCoordinate, nY + nTranslateY)) {
                    printf("Unsuccessful snprintf() in generateEmptyBoard(): halting.\n");
                    exit(EXIT_FAILURE);
                }
                /* Append line to list for output file. */
                appendToList(lstEmptyBoard, sBuffer);
                nY += SQUARE_HEIGHT;
            }
        } else {
            /* Black on bottom. */
            for (char cCoordinate = '1'; cCoordinate < '9'; cCoordinate += '1' - '0') {
                /* Generate SVG line. */
                if (!snprintf(sBuffer, BUFFER_SIZE,
                        "    <use xlink:href = \"#coordinate%c\" x = \"0\" y = \"%d\" />",
                        cCoordinate, nY + nTranslateY)) {
                    printf("Unsuccessful snprintf() in generateEmptyBoard(): halting.\n");
                    exit(EXIT_FAILURE);
                }
                /* Append line to list for output file. */
                appendToList(lstEmptyBoard, sBuffer);
                nY += SQUARE_HEIGHT;
            }
        }
        /* Horizontal coordinates (from 'a' to 'h') */
        nX = 0;
        nY = 8 * SQUARE_HEIGHT;
        nTranslateX = VERTICAL_COORDINATES_WIDTH;
        nTranslateX += BORDER_THICKNESS;
        nTranslateY = BORDER_THICKNESS;
        nTranslateY += BORDER_THICKNESS;
        if (bWhiteAtBottom) {
            /* White at bottom. */
            for (char cCoordinate = 'a'; cCoordinate < 'i'; cCoordinate += 'b' - 'a') {
                /* Generate SVG line. */
                if (!snprintf(
                        sBuffer,
                        BUFFER_SIZE,
                        "    <use xlink:href = \"#coordinate%c\" x = \"%d\" y = \"%d\" />",
                        cCoordinate,
                        nX + nTranslateX,
                        nY + nTranslateY)) {
                    printf("Unsuccessful snprintf() in generateEmptyBoard(): halting.\n");
                    exit(EXIT_FAILURE);
                };
                /* Append line to list for output file. */
                appendToList(lstEmptyBoard, sBuffer);
                nX += SQUARE_WIDTH;
            }
        } else {
            /* Black at bottom. */
            for (char cCoordinate = 'h'; cCoordinate >= 'a'; cCoordinate -= 'b' - 'a') {
                /* Generate SVG line. */
                if (!snprintf(
                        sBuffer,
                        BUFFER_SIZE,
                        "    <use xlink:href = \"#coordinate%c\" x = \"%d\" y = \"%d\" />",
                        cCoordinate,
                        nX + nTranslateX,
                        nY + nTranslateY)) {
                    printf("Unsuccessful snprintf() in generateEmptyBoard(): halting.\n");
                    exit(EXIT_FAILURE);
                };
                /* Append line to list for output file. */
                appendToList(lstEmptyBoard, sBuffer);
                nX += SQUARE_WIDTH;
            }
        }
    }

    return lstEmptyBoard;
}

/**
 * Write SVG definitions, empty board and chess pieces to a file.
 **/
bool writeListsToFile(char* sOutputFile, LinkedList lstTemplate, LinkedList lstEmptyBoard,
    LinkedList lstPieces)
{

    ListItem* itmSVGLine = NULL;
    FILE* fOutputFile;

    /* OPEN FILE */
    fOutputFile = fopen(sOutputFile, "w");
    if (fOutputFile == NULL) {
        printf("Error: cannot open output file (%s).", sOutputFile);
        return false;
    }

    /* WRITE TEMPLATE TO FILE.*/
    itmSVGLine = lstTemplate.First;
    while (itmSVGLine) {
        if (itmSVGLine->Value) {
            fprintf(fOutputFile, "%s\n", itmSVGLine->Value);
        }
        itmSVGLine = itmSVGLine->Next;
    }

    /* WRITE EMPTY BOARD TO FILE.*/
    itmSVGLine = lstEmptyBoard.First;
    while (itmSVGLine) {
        if (itmSVGLine->Value) {
            fprintf(fOutputFile, "%s\n", itmSVGLine->Value);
        }
        itmSVGLine = itmSVGLine->Next;
    }

    /* ADD PIECES TO FILE. */
    itmSVGLine = lstPieces.First;
    while (itmSVGLine) {
        if (itmSVGLine->Value) {
            fprintf(fOutputFile, "%s\n", itmSVGLine->Value);
        }
        itmSVGLine = itmSVGLine->Next;
    }

    /* WRITE SVG CLOSURE LINE. */
    fprintf(fOutputFile, "</svg>\n");

    /* CLOSE FILE */
    fclose(fOutputFile);
}

/**
 * For each FEN:
 * 1. Store SVG definitions of board and pieces in a linked list.
 * 2. Use those definitions to define an empty board in a linked list.
 * 3. Use those definitions to fill the board with pieces in a linked list.
 * 4. Write down those three linked list to a file.
 **/
bool writeDiagramsToFiles(LinkedList lstTemplate, LinkedList lstNormalEmptyBoard,
    LinkedList lstReversedEmptyBoard, LinkedList lstFEN, bool bBorder, bool bCoordinates,
    bool bMoveIndicator, bool bPositionAsFileName, bool bRotateBoard)
{

    /* */
    LinkedList lstEmptyBoard; /* Received as parameter. */
    LinkedList* lstPieces = NULL;
    ListItem* itmFEN = NULL; /* Current item in the list of FEN position. */
    char* sFileName = NULL;

    /* BROWSE THE LIST OF FEN */
    int nDiagramNumber = 1;
    itmFEN = lstFEN.First;
    while (itmFEN) { /*For each FEN... */
        if (itmFEN->Value) {

            /* WHICH EMPTY BOARD TO USE (White or Black at bottom)? */
            if (isWhiteToPlay(itmFEN->Value) || !bRotateBoard) {
                lstEmptyBoard = lstNormalEmptyBoard;
            } else {
                lstEmptyBoard = lstReversedEmptyBoard;
            }

            /* FILL BOARD WITH PIECES. */
            lstPieces = createPieces(itmFEN->Value, bBorder, bCoordinates, bMoveIndicator,
                bRotateBoard);

            /* GENERATE FILE NAME */
            if (bPositionAsFileName) {
                sFileName = generateFENFileName(itmFEN->Value);
            } else {
                sFileName = generateNumberedFileName(nDiagramNumber++);
            }

            /* WRITE BOARD AND PIECES TO FILE. */
            writeListsToFile(sFileName, lstTemplate, lstEmptyBoard, *lstPieces);

            /* GET NEXT FEN */
            free(sFileName);
            freeList(&lstPieces);
            itmFEN = itmFEN->Next;
        }
    }

    return true;
}

/** Self-explanatory. **/
int main(int argc, char* argv[])
{

    /*
    char* sFileName = generateFileName("r2qkbnr/ppp2ppp/3p4/4n3/2B1P1b1/2N2N2/PP3PPP/R1BQ1RK1 w kq - 4 8");
    printf("%s\n", sFileName);
    free(sFileName);

    return EXIT_SUCCESS;
    */

    enum InputMode { UNKNOWN_MODE,
        FILE_MODE,
        STRING_MODE }; /* Do we receive the
FEN strings directly or are they stored in a file? */

    enum InputMode enuInputMode = UNKNOWN_MODE;
    bool bBorder = false;
    bool bCoordinates = false;
    bool bMoveIndicator = false;
    bool bRotateBoard = false;
    bool bPositionAsFileName = false;

    /* 1 - PARSE COMMAND LINE ARGUMENTS */
    /* No argument? */
    if (argc == 1) {
        fprintf(stderr, "%s: missing operand\n", argv[0]);
        fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    /* Process arguments one by one. */
    int c;
    while ((c = getopt(argc, argv, "hbcmprfs")) != -1) {
        switch (c) {
        case 'h':
            /* Display help */
            printf("FEN2SVG v0.3 (May 29 2023): create nice SVG diagrams from FEN strings.\n");
            printf("This program is free software: you can redistribute "
                   "and/or modify it.\n");
            printf("FEN2SVG comes with ABSOLUTELY NO WARRANTY, to the "
                   "extent permitted by \n");
            printf("applicable law.\n");
            printf("Written by Michaël I. F. George.\n");
            printf("License GPLv3+: GNU GPL version 3 or later \n");
            printf("<http://gnu.org/licenses/gpl.html>\n");
            printf("\n");
            fprintf(stderr, "Usage: %s [-bcmrfs] file(s) or string(s)\n",
                argv[0]);
            printf("    -b\tborders\n");
            printf("    -c\texternal coordinates\n");
            printf("    -m\tmove indicator\n");
            printf("    -p\tposition (i.e. FEN) as file name\n");
            printf("    -r\trotate board (i.e. side to move below)\n");
            printf("    -f\tfile mode (default):\n");
            printf("    \tFEN positions are contained in a file\n");
            printf("    -s\tstring mode:\n");
            printf("    \tFEN posititions are passed directly in the command "
                   "line\n");
            return EXIT_SUCCESS;
            break;
        case 'b':
            bBorder = true;
            break;
        case 'c':
            bCoordinates = true;
            break;
        case 'm':
            bMoveIndicator = true;
            break;
        case 'r':
            bRotateBoard = true;
            break;
        case 'p':
            bPositionAsFileName = true;
            break;
        case 'f':
            /* Store input file name */
            if (enuInputMode != UNKNOWN_MODE && enuInputMode != FILE_MODE) {
                fprintf(stderr, "%s: only one input mode (string or file) "
                                "can be selected at a time\n",
                    argv[0]);
                fprintf(stderr, "Try '%s -h' for more information.\n",
                    argv[0]);
                exit(EXIT_FAILURE);
            } else {
                enuInputMode = FILE_MODE;
            }

            // sInputFile = optarg;
            break;
        case 's':
            /* Store FEN string */
            if (enuInputMode != UNKNOWN_MODE && enuInputMode != STRING_MODE) {
                fprintf(stderr, "%s: only one input mode (string or file) "
                                "can be selected at a time\n",
                    argv[0]);
                fprintf(stderr, "Try '%s -h' for more information.\n",
                    argv[0]);
                exit(EXIT_FAILURE);
            } else {
                enuInputMode = STRING_MODE;
            }
            // sInputFEN = optarg;
            break;
        case '?':
            break;
        default:
            fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
        }
    }

    /* Set file as default input mode. */
    if (enuInputMode == UNKNOWN_MODE) {
        enuInputMode = FILE_MODE;
    }

    /* Non-optional arguments (options are preceeded by a '-')
        Here FEN file(s) or string(s) are expected. */
    LinkedList* lstArgument = createEmptyList(); /* File names or FEN strings */
    if (optind >= argc) {
        fprintf(stderr, "%s: no file or FEN string to process\n", argv[0]);
        fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
        exit(EXIT_FAILURE);
    } else {
        while (optind < argc) { /* optind = index value of the next argument */
            appendToList(lstArgument, argv[optind++]);
        }
    }

    /* 2 - READ SVG TEMPLATE (contains definitions for board items and pieces) */
    LinkedList* lstTemplate = readTemplate(SVG_TEMPLATE);
    if (!addLengthsToTemplate(*lstTemplate, bBorder, bCoordinates, bMoveIndicator)) {
        freeList(&lstTemplate);
        return EXIT_FAILURE;
    }
    /* 3 - GENERATE TWO EMPTY CHESSBOARDS (same boards are used for every position) */
    /* White at bottom. */
    LinkedList* lstNormalEmptyBoard = generateEmptyBoard(bBorder, bCoordinates, bMoveIndicator, WHITE_ON_BOTTOM);
    /* Black at bottom. */
    LinkedList* lstReversedEmptyBoard = generateEmptyBoard(bBorder, bCoordinates, bMoveIndicator, BLACK_ON_BOTTOM);

    /* 3 - READ INPUT FEN STRINGS. */
    LinkedList* lstFEN = createEmptyList();

    if (enuInputMode == FILE_MODE) {
        /* Get FEN strings from one or several files. */
        ListItem* lstCurrent = (*lstArgument).First;
        while (lstCurrent) {
            if (lstCurrent->Value) {
                readFENFile(lstCurrent->Value, lstFEN);
            }
            lstCurrent = lstCurrent->Next;
        }
    } else {
        /* Get FEN strings directly from the command line. */
        ListItem* lstCurrent = (*lstArgument).First;
        while (lstCurrent) {
            if (lstCurrent->Value) {
                appendToList(lstFEN, lstCurrent->Value);
            }
            lstCurrent = lstCurrent->Next;
        }
    }
    /* Free memory. */
    freeList(&lstArgument);

    /* 4 - FILL AND WRITE DOWN SVG DIAGRAMS. */
    writeDiagramsToFiles(*lstTemplate, *lstNormalEmptyBoard, *lstReversedEmptyBoard, *lstFEN,
        bBorder, bCoordinates, bMoveIndicator, bPositionAsFileName, bRotateBoard);

    /* 5 - FREE MEMORY. */
    freeList(&lstTemplate);
    freeList(&lstFEN);
    freeList(&lstNormalEmptyBoard);
    freeList(&lstReversedEmptyBoard);

    /* 6 - RETURN "EVERYTHING WENT WELL". */
    return EXIT_SUCCESS;
}
