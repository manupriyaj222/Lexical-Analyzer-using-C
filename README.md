# Lexical Analyzer Using C

## Project Overview

The Lexical Analyzer is a C programming application that reads a source code file and identifies different types of tokens.

It analyzes the input character by character and classifies tokens such as keywords, identifiers, numeric constants, operators, string literals, character literals, comments and special symbols.

It also maintains a symbol table containing unique identifiers found in the source code.

## Features

- Recognizes C keywords.
- Identifies variable and function identifiers.
- Recognizes numeric and floating-point constants.
- Identifies arithmetic and relational operators.
- Recognizes string and character literals.
- Detects single-line and multi-line comments.
- Identifies preprocessor directives.
- Recognizes parentheses, curly braces and square brackets.
- Identifies separators and semicolons.
- Maintains a symbol table of unique identifiers.
- Detects unterminated string and character literals and block comments.

## Technologies Used

- Programming Language: C
- Compiler: GCC
- Libraries: stdio.h, ctype.h, string.h
- Concepts: File Handling, Strings, Arrays, Functions, Pointers, Character Processing.

## Project Structure

```text
Lexical-Analyzer-using-C/
│
├── edit.c
├── README.md
└── .gitignore
```

## Compilation

Compile the program using GCC:

```bash
gcc edit.c -o lexical_analyzer
```

## Execution

Run the program:

```bash
./lexical_analyzer
```

The program prompts you to enter the source file name.

Example:

```text
Enter source file name: sample.c
```

Make sure the input C source file is available in the current working directory.

## Sample Input

```c
#include <stdio.h>

int main()
{
    int number = 10;

    printf("Hello World");

    return 0;
}
```

## Sample Output

```text
#include <stdio.h> : Preprocessor Directive (Include)

int : Keyword
main : Identifier
( : Opening Parenthesis
) : Closing Parenthesis
{ : Opening Curly Brace
int : Keyword
number : Identifier
= : Operator
10 : Numeric Constant
; : Semicolon (Statement Terminator)
"Hello World" : String Literal
return : Keyword
0 : Numeric Constant
; : Semicolon (Statement Terminator)
} : Closing Curly Brace

====== SYMBOL TABLE ======
main
number
```

## Symbol Table

The program maintains a symbol table that stores unique identifiers encountered during lexical analysis.

Duplicate identifiers are not added again.

## Learning Outcomes

- Understanding the fundamentals of lexical analysis.
- Identifying and classifying different token types.
- Understanding C keywords and identifiers.
- Working with file handling and character processing.
- Implementing string comparison and manipulation.
- Understanding symbol table implementation.
- Improving logical thinking and problem-solving skills.

## Author

Manupriya J

## Project Domain

C Programming | Compiler Design | Lexical Analysis | File Handling
