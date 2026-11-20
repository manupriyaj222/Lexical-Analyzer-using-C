#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN 512
#define MAX_IDENTIFIERS 500
#define MAX_IDENTIFIER_LENGTH 128

/* Expanded keyword list to recognize common C and C11 keywords. */
static const char *keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default",
    "do", "double", "else", "enum", "extern", "float", "for",
    "goto", "if", "inline", "int", "long", "register", "restrict",
    "return", "short", "signed", "sizeof", "static", "struct",
    "switch", "typedef", "union", "unsigned", "void", "volatile",
    "while"
};

/* C replacement for the C++ set used by the reference implementation. */
static char symbolTable[MAX_IDENTIFIERS][MAX_IDENTIFIER_LENGTH];
static size_t symbolCount = 0;

/* Check whether a word is a C keyword. */
static int isKeyword(const char *word)
{
    size_t count = sizeof(keywords) / sizeof(keywords[0]);
    size_t i;

    for (i = 0; i < count; i++) {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }

    return 0;
}

/* Add unique identifiers to the symbol table. */
static void addIdentifier(const char *word)
{
    size_t i;

    for (i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i], word) == 0)
            return;
    }

    if (symbolCount < MAX_IDENTIFIERS) {
        strncpy(symbolTable[symbolCount], word,
                MAX_IDENTIFIER_LENGTH - 1);

        symbolTable[symbolCount][MAX_IDENTIFIER_LENGTH - 1] = '\0';
        symbolCount++;
    } else {
        fprintf(stderr, "Warning: Symbol table is full.\n");
    }
}

/* Read identifiers that may start with a letter or underscore. */
static void readIdentifier(FILE *file, int firstChar,
                           char *word, size_t size)
{
    size_t length = 0;
    int ch = firstChar;

    if (size == 0)
        return;

    word[length++] = (char)ch;

    while ((ch = fgetc(file)) != EOF &&
           (isalnum((unsigned char)ch) || ch == '_')) {
        if (length + 1 < size)
            word[length++] = (char)ch;
    }

    if (ch != EOF)
        ungetc(ch, file);

    word[length] = '\0';
}

/* Read string and character literals, including escaped quotes. */
static void readQuotedLiteral(FILE *file, int quote,
                              char *word, size_t size)
{
    size_t length = 0;
    int ch;
    int closed = 0;

    if (size == 0)
        return;

    word[length++] = (char)quote;

    while ((ch = fgetc(file)) != EOF) {
        if (length + 1 < size)
            word[length++] = (char)ch;

        /* An escape character protects the next character. */
        if (ch == '\\') {
            ch = fgetc(file);

            if (ch == EOF)
                break;

            if (length + 1 < size)
                word[length++] = (char)ch;
        } else if (ch == quote) {
            closed = 1;
            break;
        } else if (ch == '\n') {
            break;
        }
    }

    word[length] = '\0';

    if (!closed) {
        printf("%s : Unterminated %s Literal\n", word,
               quote == '"' ? "String" : "Character");
    } else {
        printf("%s : %s Literal\n", word,
               quote == '"' ? "String" : "Character");
    }
}

/* Recognize preprocessor directives from the complete source line. */
static void readPreprocessorLine(FILE *file, int firstChar)
{
    char line[MAX_TOKEN];
    size_t length = 0;
    int ch = firstChar;
    const char *label = "Preprocessor Directive";

    line[length++] = (char)ch;

    while ((ch = fgetc(file)) != EOF && ch != '\n') {
        if (length + 1 < sizeof(line))
            line[length++] = (char)ch;
    }

    line[length] = '\0';

    /* Check directive names without matching longer unrelated names. */
    if (strncmp(line, "#include", 8) == 0 &&
        (line[8] == '\0' || isspace((unsigned char)line[8]) ||
         line[8] == '<' || line[8] == '"'))
        label = "Preprocessor Directive (Include)";
    else if (strncmp(line, "#define", 7) == 0 &&
             (line[7] == '\0' || isspace((unsigned char)line[7])))
        label = "Preprocessor Directive (Macro Definition)";
    else if (strncmp(line, "#undef", 6) == 0 &&
             (line[6] == '\0' || isspace((unsigned char)line[6])))
        label = "Preprocessor Directive (Macro Undefinition)";
    else if (strncmp(line, "#ifdef", 6) == 0 &&
             (line[6] == '\0' || isspace((unsigned char)line[6])))
        label = "Preprocessor Directive (If Defined)";
    else if (strncmp(line, "#ifndef", 7) == 0 &&
             (line[7] == '\0' || isspace((unsigned char)line[7])))
        label = "Preprocessor Directive (If Not Defined)";
    else if (strncmp(line, "#if", 3) == 0 &&
             (line[3] == '\0' || isspace((unsigned char)line[3])))
        label = "Preprocessor Directive (Conditional Compilation)";
    else if (strncmp(line, "#elif", 5) == 0 &&
             (line[5] == '\0' || isspace((unsigned char)line[5])))
        label = "Preprocessor Directive (Else If)";
    else if (strncmp(line, "#else", 5) == 0 &&
             (line[5] == '\0' || isspace((unsigned char)line[5])))
        label = "Preprocessor Directive (Else)";
    else if (strncmp(line, "#endif", 6) == 0 &&
             (line[6] == '\0' || isspace((unsigned char)line[6])))
        label = "Preprocessor Directive (End If)";
    else if (strncmp(line, "#pragma", 7) == 0 &&
             (line[7] == '\0' || isspace((unsigned char)line[7])))
        label = "Preprocessor Directive (Pragma)";
    else if (strncmp(line, "#line", 5) == 0 &&
             (line[5] == '\0' || isspace((unsigned char)line[5])))
        label = "Preprocessor Directive (Line Control)";
    else if (strncmp(line, "#error", 6) == 0 &&
             (line[6] == '\0' || isspace((unsigned char)line[6])))
        label = "Preprocessor Directive (Error)";

    printf("%s : %s\n", line, label);
}

/* Recognize common integer and floating-point constants. */
static void readNumber(FILE *file, int firstChar,
                       char *word, size_t size)
{
    size_t length = 0;
    int ch;
    int isFloating = 0;

    if (size == 0)
        return;

    word[length++] = (char)firstChar;

    while ((ch = fgetc(file)) != EOF) {
        if (isalnum((unsigned char)ch) ||
            ch == '_' || ch == '.') {
            if (ch == '.')
                isFloating = 1;

            if (length + 1 < size)
                word[length++] = (char)ch;
        } else if ((ch == '+' || ch == '-') && length > 0 &&
                   (word[length - 1] == 'e' ||
                    word[length - 1] == 'E' ||
                    word[length - 1] == 'p' ||
                    word[length - 1] == 'P')) {
            if (length + 1 < size)
                word[length++] = (char)ch;
        } else {
            ungetc(ch, file);
            break;
        }
    }

    word[length] = '\0';

    printf("%s : %s Constant\n", word,
           isFloating ? "Floating" : "Numeric");
}

/* Recognize common single- and double-character operators. */
static void printOperator(FILE *file, int ch)
{
    int next = fgetc(file);
    char op[3] = {(char)ch, '\0', '\0'};

    if ((ch == '+' && (next == '+' || next == '=')) ||
        (ch == '-' && (next == '-' || next == '=' || next == '>')) ||
        (ch == '*' && next == '=') ||
        (ch == '/' && next == '=') ||
        (ch == '%' && next == '=') ||
        (ch == '=' && next == '=') ||
        (ch == '!' && next == '=') ||
        (ch == '<' && (next == '=' || next == '<')) ||
        (ch == '>' && (next == '=' || next == '>')) ||
        (ch == '&' && (next == '&' || next == '=')) ||
        (ch == '|' && (next == '|' || next == '=')) ||
        (ch == '^' && next == '=')) {
        op[1] = (char)next;
        printf("%s : Operator\n", op);
    } else {
        if (next != EOF)
            ungetc(next, file);

        printf("%c : Operator\n", ch);
    }
}

int main(void)
{
    char filename[256];
    FILE *file;
    int ch;
    char word[MAX_TOKEN];

    printf("Enter source file name: ");

    if (scanf("%255s", filename) != 1) {
        fprintf(stderr, "Error: No source file name provided.\n");
        return 1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error: Unable to open file");
        return 1;
    }

    printf("\n====== LEXICAL ANALYZER OUTPUT ======\n\n");

    while ((ch = fgetc(file)) != EOF) {
        /* Cast to unsigned char for safe use of ctype functions. */
        if (isspace((unsigned char)ch))
            continue;

        if (ch == '#') {
            /* Added: identify preprocessor directives. */
            readPreprocessorLine(file, ch);
        } else if (ch == '"') {
            /* Added: recognize string literals. */
            readQuotedLiteral(file, ch, word, sizeof(word));
        } else if (ch == '\'') {
            /* Added: recognize character literals. */
            readQuotedLiteral(file, ch, word, sizeof(word));
        } else if (isalpha((unsigned char)ch) || ch == '_') {
            readIdentifier(file, ch, word, sizeof(word));

            if (isKeyword(word)) {
                printf("%s : Keyword\n", word);
            } else {
                printf("%s : Identifier\n", word);
                addIdentifier(word);
            }
        } else if (isdigit((unsigned char)ch)) {
            readNumber(file, ch, word, sizeof(word));
        } else if (ch == '/') {
            int next = fgetc(file);

            if (next == '/') {
                /* Added: recognize single-line comments. */
                while ((ch = fgetc(file)) != EOF && ch != '\n')
                    ;

                printf("// : Single Line Comment\n");
            } else if (next == '*') {
                /* Added: recognize block comments and detect missing end. */
                int previous = 0;
                int closed = 0;

                while ((ch = fgetc(file)) != EOF) {
                    if (previous == '*' && ch == '/') {
                        closed = 1;
                        break;
                    }

                    previous = ch;
                }

                if (closed)
                    printf("/* */ : Multi Line Comment\n");
                else
                    printf("/* : Unterminated Multi Line Comment\n");
            } else {
                if (next != EOF)
                    ungetc(next, file);

                printOperator(file, ch);
            }
        } else if (strchr("+-*%=!<>&|^~?:", ch) != NULL) {
            printOperator(file, ch);
        } else if (ch == '(') {
            printf("( : Opening Parenthesis\n");
        } else if (ch == ')') {
            printf(") : Closing Parenthesis\n");
        } else if (ch == '{') {
            printf("{ : Opening Curly Brace\n");
        } else if (ch == '}') {
            printf("} : Closing Curly Brace\n");
        } else if (ch == '[') {
            printf("[ : Opening Square Bracket\n");
        } else if (ch == ']') {
            printf("] : Closing Square Bracket\n");
        } else if (ch == ';') {
            printf("; : Semicolon (Statement Terminator)\n");
        } else if (ch == ',') {
            printf(", : Comma (Separator)\n");
        } else if (ch == '.') {
            printf(". : Member/Structure Access Operator\n");
        } else {
            /* Retained: report unrecognized characters. */
            printf("%c : Invalid Symbol\n", ch);
        }
    }

    fclose(file);

    /* Print the unique identifiers collected during tokenization. */
    printf("\n====== SYMBOL TABLE ======\n");

    {
        size_t i;

        for (i = 0; i < symbolCount; i++)
            printf("%s\n", symbolTable[i]);
    }

    return 0;
}