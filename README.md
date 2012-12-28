C compiler using Flex and Bison
===============================
##How to build
`make`

##Run
With a file that needs preprocessing (has `#include`s, `#define`s, etc):

    gcc -E inputfile.c | ./a

Otherwise, from some file:

    cat inputfile.c | ./a

Or just run `./a` and type in your code manually.

###Options
* `-a`: show AST
* `-d`: show declarations
* `-q`: show quads
* `-t`: show target code (x86)
* `-T`: same as `-t` but also show the quads as comments in target code

Note: if none of the above options are specified, `-t` will be set by default.

