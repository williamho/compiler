C compiler using Flex and Bison
===============================
##Build
`make`

##Run
With a file that needs preprocessing (has `#include`s, `#define`s, etc):

    gcc -E inputfile.c | ./compile >outputfile.s

Otherwise, from some file:

    ./compile <inputfile.c >outputfile.s

Or just run `./compile >outputfile.s` and type in your code manually.

The `outputfile.s` file contains x86 assembly (AT&T syntax). To create an 
executable and run it:

	gcc -m32 outputfile.s -o a.out
	./a.out

###Options
* `-a`: show AST
* `-d`: show declarations
* `-q`: show quads
* `-t`: show target code (x86)
* `-T`: same as `-t` but also show the quads as comments in target code

Note: if none of the above options are specified, `-t` will be set by default.

For sample input/output files, see the `tests` directory.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

##Features

* show AST, declarations, quads, and target code (x86)
* reading and writing local and global variables (including block scopes)
	* `int`s and `int` pointers/arrays only
* computation of expressions (most of them)
* call functions with arguments 
	* including external ones such as `printf`, assuming linked
* string literals (necessary to get `printf` working)
* pointers and pointer indirection
	* array access
	* pointer arithmetic
* most control flow statements (see anti-features section below)

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

##Anti-features
What this compiler doesn't support:

* certain declarations
	* type specifiers that aren't `int` or pointers to / arrays of `int`s
	* type qualifiers (`const`, `volatile`, `restrict`)
	* initialized declarations
	* storage classes
	* anything to do with `struct`s or `union`s
	* `enum`s
	* `typedef`s?
	* functions that take parameters
	* multi-dimensional arrays
* certain expressions
	* `sizeof`
	* casts
	* ternary operator (i.e. `a ? b : c`)
	* short circuit operators `&&` and `||`
		* the operators exist and return 0 or 1; they just don't short circuit
	* division of pointers
	* unary `+` operator
		* what does this even do
* certain control flow statements
	* `do` loops
	* `switch`es
	* `goto` statements
	* labels
* handling of multiple input files hasn't been tested

Things that it does now that could be improved:

* no register allocation
	* currently just uses a separate local variable for each temporary var
* memory leaks everywhere 
	* hasn't been a problem yet
	* just don't use this to compile any large and useful programs
* short circuit operators don't actually short circuit
* terribly disorganized
	* do the AST nodes properly!
* quads are internal (the printed quads aren't actually used as input)
* sizes are hardcoded to 4, since only `int`s/pointers are supported

