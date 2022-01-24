# FEN2SVG
Creates very quickly chess diagrams (in SVG) from a file containing chess positions (i.e. FEN strings).

# Introduction

## What is this piece of software intented for?
It receives a FEN string or a file of FEN strings and transform it to chess diagram(s), in SVG.

A lot of diagrams can be generated in no time.

For example, 

`rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3`

produces

![SVG produced](https://github.com/michael-i-f-george/FEN2SVG/blob/master/example.svg "example.svg")




## How do I use it?

1. Copy-paste FEN positions in a text file.
2. Give this text file the .fen extension (e.g. mychesspositions.fen).
3. Launch it.
     1. If you are using Linux: `./fen2svg -bcm mychesspositions.fen`, where
        * `b` stands for borders,
        * `c` stands for coordinates,
        * `m` stands for move indicator.
     2. If your are using Windows, in the command prompt: `fen2svg.exe -bcm mychesspositions.fen`, where
        * `b` stands for borders,
        * `c` stands for coordinates,
        * `m` stands for move indicator.


## What does FEN means?

From Wikipedia, the free encyclopedia

> Forsyth–Edwards Notation (FEN) is a standard notation for describing a particular board position of a chess game. The purpose of FEN is to provide all the necessary information to restart a game from a particular position.

### Examples

Here is the FEN for the starting position:

`rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1`

Here is the FEN after the move 1. e4:

`rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1`


## What does SVG means?

From Wikipedia, the free encyclopedia

> Scalable Vector Graphics (SVG) is an Extensible Markup Language (XML)-based vector image format for two-dimensional graphics with support for interactivity and animation. The SVG specification is an open standard developed by the World Wide Web Consortium (W3C) since 1999.

> All major modern web browsers—including Mozilla Firefox, Internet Explorer, Google Chrome, Opera, Safari, and Microsoft Edge—have SVG rendering support. 

## Good to know
There is no real check of FEN legality.
* Row separators (“/”) are just ignored.
* If less than 64 squares are found, the chessboard is filled with empty squares.
* If more than 64 squares are found, the surplus is simply ignored.

The main benefit of doing so is that the user has less constraints. Thus:
* the board can be empty or completely full (up to 64 pieces),
* there can be any number pieces of the same kind,
* kings are not mandatory,
* pawns can stand on the first (eighth) rank,
* both kings can be in check at the same time,
* ...

You are free!

# Implementation

<details>
<summary>Are you sure that you want to read about this? :-)</summary>

## Tools used
* **SVG editor**: Inkscape and Geany,
* **SVG cleaner**: SVGO,
* **chessmen**: [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Chess_Pieces_Sprite.svg), [Lichess](https://github.com/ornicar/lila/tree/master/public/piece/staunty),
* **C editor**: Geany,
* **C compilator**: GCC (Linux), MinGW (Windows),
* **C debugger**: Valgrind, GDB,
* **C validator**: [Splint](https://splint.org/),
* **documentation editor**: Texmaker,
* **absolute to relative paths**:
  * [SVG transformations by Peter Collingridge](http://petercollingridge.appspot.com/svg_transforms/),
  * https://github.com/Klowner/inkscape-applytransforms.

## SVG
### Why SVG?
* Diagrams are **scalable**: the image can be resized infinitely without losing any quality.
* Diagrams can be **customized**: you can design a template of your own (wood squares, another move indicator, ...).
* You can **modify the output drawing** (add an arrow, highlight or surround squares, ...).
* It is usable within **web-browser** and **EPUB**.
* A vector graphic can be converted into another format (e.g. PNG) and serve as a clean reference.
* You know exactly how items are draw.

### How is the base template structured?
FEN2SVG uses a file where all the items are defined: template.svg.

There are two main parts:
* the `<defs>` `</defs>`, were are defined
  * pieces,
  * coordinates,
  * squares,
  * border,
  * move indicator.
* the below part, where those defined items are used.

### What is the square size?
Board is made of 64 squares. Each one measures 72 x 72.

### Which font is used in the base template (for coordinates)?
Ume Gothic L

### Why did you convert your coordinates font to path?
To avoid missing font issues.

### Can I customize my own template?
Of course, you can.

Do absolutly not modify the first (`<svg`) and last line (`</svg>`), respect the definitions structure.

If you think it is worth it, share the result of your work.

### How are the pieces aligned on their square?
Because queen is the tallest piece, it is used as reference. Queen is centered on its square Other pieces align their
bottom with the queen bottom.

### Why not SVGZ?
The goal is to keep FEN2SVG as simple as possible.

SVGZ are compressed SVG files. Therefore, to create SVGZ, two steps are required:
1. create the SVG (as it is currently the case),
2. compress it (where a C library is needed).

The good news is that you still can compress it by yourself, if needed.


## Source code
### Why has C language been used for development?
C language is fast, lightweight and portable, moreover no graphical user interface is needed. The main drawback is
precisly the lack of an interface.
### How to compile for Linux?
`gcc fen2svg.c unsortedlinkedlist.c -o fen2svg`

### Detecting memory leaks under Linux
Follow these two steps, in that order:
1. `gcc -g -o0 unsortedlinkedlist.c fen2svg.c -o fen2svg`
2. `valgrind -v --leak-check=full ./fen2svg`

[GDB (GNU Debugger)](https://www.gnu.org/software/gdb/) could also prove useful.

### How to validate code under Linux?
`splint unsortedlinkedlist.c fen2svg.c`

### How to compile for Windows under Linux?
* **32 bit**: `i686-w64-mingw32-gcc fen2svg.c unsortedlinkedlist.c -o fen2svg.exe`
* **64 bit**: `x86_64-w64-mingw32-gcc fen2svg.c unsortedlinkedlist.c -o fen2svg.exe`

### How to compile for Windows under Windows?
Use your favourite compiler/IDE. If you have none, [Mingw-w64](https://mingw-w64.org/) could be worth a try.

Adapt the instructions from the above section.

# Possible improvements
* add a friendly-user interface,
* handle correctly unexpected end of files,
* allow to check every FEN string (strict mode),
* permit SVGZ compression,
* allow to change square colour,
* change square texture (e.g. wood),
* add arrows, circles, squares and the like,
* limit output to a given area of the board,
* mark squares with dots, cross, . . . ,
* translucid squares,
* change move indicator,
* allow inner coordinates (rather than outside the board),
* allow fairy chess,
* use gradients for pieces or chessboard,
* add a caption at diagram bottom.

</details>

# Contact

Feel free to contact me at [fen2svg.to.stronghold@spamgourmet.com](mailto:fen2svg.to.stronghold@spamgourmet.com), for improvements, corrections or sharing SVG
chessboard templates.
