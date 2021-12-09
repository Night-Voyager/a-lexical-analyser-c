#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#define IDENTIFIER_MAX_LEN 32

static char keywords[][11] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float",
    "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", "struct", "switch",
    "typeof", "union", "unsigned", "void", "volatile", "while",  // 32 keywords for ANSI C
    "_Bool", "_Complex", "_Imaginary", "inline", "restrict"  // 5 new keywords for C99
};

static char operators[] = {
    '+', '-', '*', '/', '%', '<', '>', '=', '!', '&', '|', '=', '^', '~'//, ',', '?'
};

static char preprocessorDirectives[][9] = {
    "#define", "#undef", "#include", "#ifdef", "#endif", "#ifndef", "#if", "#else", "#elif", "#pragma", "#error"
};

FILE * file;
char currentChar;

char token[100] = {'\0'};
int token_length = 0;

int row = 1;
int col = 0;
int length_of_previous_row;
int token_start_row;
int token_start_col;

int isKeyword(char *);
int isOperator(char);
int isPreprocessorDirective(char *);
void handleComments();
void handlePunctuations();
void handleConstants();
void handleKeywordsAndIdentifiers();
void printErrorOrWarning(int, char *, ...);
char getChar();
void resetCursor();

int main(int argc, char * argv[]) {
    if (argc == 1)
        file = fopen("../test.c", "r");
    else
        file = fopen(argv[1], "r");

    if (file == NULL)
        printErrorOrWarning(1, "file \"%s\" is not found", argv[1]);

    while ( (currentChar = getChar()) != EOF ) {
        if (!isspace(currentChar)) {
            // initialize token buffer
            token[0] = '\0';
            token_length = 0;

            // save the location in the file where a token starts
            token_start_row = row;
            token_start_col = col;

            // handle constants
            if (isdigit(currentChar) || currentChar == '\'' || currentChar == '\"') {
                handleConstants(currentChar);
                continue;
            }

            // handle keywords and identifiers
            if (isalpha(currentChar) || currentChar == '_') {
                handleKeywordsAndIdentifiers(currentChar);
                continue;
            }

            // handle punctuations
            if (ispunct(currentChar)) {
                handlePunctuations(currentChar);
                continue;
            }
        }
    }

    fclose(file);
    return 0;
}

int isKeyword(char * s) {
    for (int i = 0; i < 37; ++i) {
        if (strcmp(s, keywords[i]) == 0) return 1;
    }
    return 0;
}

int isOperator(char c) {
    for (int i = 0; i < 16; ++i) {
        if (c == operators[i]) return 1;
    }
    return 0;
}


int isPreprocessorDirective(char * s) {
    for (int i = 0; i < 11; ++i) {
        if (strcmp(s, preprocessorDirectives[i]) == 0) return 1;
    }
    return 0;
}

void handleComments() {
    switch (currentChar) {
        case '/':
            while ( (currentChar = getChar()) != '\n' );
            break;
        case '*':
            while ( (currentChar = getChar()) != '*' || (currentChar = getChar()) != '/' );
            break;
    }
}

void handleConstants() {
    switch (currentChar) {
        case '\'':  // handle single character
            do {
                token[token_length++] = currentChar;
                if (currentChar == '\\')
                    token[token_length++] = getChar();
            } while ( (currentChar = getChar()) != '\'' && currentChar != '\r' && currentChar != '\n');

            token[token_length++] = currentChar;
            token[token_length] = '\0';

            // check for the terminating ' character
            if (currentChar != '\'') {
                printErrorOrWarning(1, "error: missing terminating ' character\n");
                return;
            }

            // check for the length of the token
            if ( (token[1] == '\\' && token_length > 4) || (token[1] != '\\' && token_length > 3) ) {
                printErrorOrWarning(0, "warning: multi-character character constant\n");
                return;
            }

            printf("<char, %s>\n", token);

            break;

        case '\"':  // handle string
            do {
                token[token_length++] = currentChar;
                if (currentChar == '\\')
                    token[token_length++] = getChar();
            } while ( (currentChar = getChar()) != '\"' && currentChar != '\r' && currentChar != '\n');

            token[token_length++] = currentChar;
            token[token_length] = '\0';
            token_length = 0;

            if (currentChar == '\r' || currentChar == '\n') {
                printErrorOrWarning(1, "error: missing terminating \" character\n");
                return;
            }

            printf("<string, %s>\n", token);

            break;
        default:  // handle numbers, including integers and floats
                  // TODO: handle octal, hexadecimal, unsigned, and long numbers
            while (isdigit(currentChar)) {
                token[token_length++] = currentChar;
                currentChar = getChar();
            }

            if (currentChar == '.') {
                token[token_length++] = currentChar;
                currentChar = getChar();
                while (isdigit(currentChar)) {
                    token[token_length++] = currentChar;
                    currentChar = getChar();
                }
            }

            token[token_length] = '\0';
            token_length = 0;
            resetCursor();

            printf("<num, %s>\n", token);
    }
}

void handlePunctuations(){
    if (isOperator(currentChar)) {
        switch (currentChar) {
            case '+':  // handle positive numbers
            case '-':  // handle negative numbers
            {
                char nextChar = getChar();
                if (isdigit(nextChar)) {
                    token[token_length++] = currentChar;
                    handleConstants(nextChar);
                    return;
                } else
                    resetCursor();  // reset the cursor for reading one more character

                break;
            }
            case '/':  // handle comments
            {
                char nextChar = getChar();
                if (nextChar == '/' || nextChar == '*') {
                    handleComments(nextChar);
                    return;
                } else
                    resetCursor();  // reset the cursor for reading one more character

                break;
            }
        }
        printf("<op, %c>\n", currentChar);
    }
    else {
        switch (currentChar) {
            case '#':  // handle preprocessor directives
                do {
                    token[token_length++] = currentChar;
                } while (isalpha(currentChar = getChar()) && token_length < IDENTIFIER_MAX_LEN);

                if (token_length == IDENTIFIER_MAX_LEN) token_length--;
                token[token_length] = '\0';
                token_length = 0;

                if (isPreprocessorDirective(token))
                    printf("<preprocessor directive, %s>\n", token);
                else
                    printErrorOrWarning(1, "error: invalid preprocessing directive %s\n", token);

                break;
            default:
                printf("<special symbol, %c>\n", currentChar);
        }
    }

    token_length = 0;
}

void handleKeywordsAndIdentifiers() {
    while ( (isalnum(currentChar) || currentChar == '_') && token_length < IDENTIFIER_MAX_LEN ) {
        token[token_length++] = currentChar;
        currentChar = getChar();
    }
    token[token_length] = '\0';
    token_length = 0;
    resetCursor();
    if (isKeyword(token)) {
        if (strcmp(token, "sizeof") == 0)
            printf("<op, sizeof>\n");
        else
            printf("<keyword, %s>\n", token);
    }
    else
        printf("<id, %s>\n", token);
}

void printErrorOrWarning(int type, char * message, ...) {
    switch (type) {
        case 0:  // yellow for warning
            system("color 6");
            break;
        case 1:  // red for error
            system("color 4");
            break;
        default:
            return;
    }

    if (strncmp(message, "file \"", 6) != 0)
        printf("row %d, column %d: ", token_start_row, token_start_col);

    va_list vaList;
    va_start(vaList, message);
    vprintf(message, vaList);
    va_end(vaList);

    system("color 7");  // resume the color
}

char getChar() {
    if (currentChar == '\n') {
        row++;
        length_of_previous_row = col;
        col = 1;
    } else {
        col++;
    }
    return getc(file);
}

void resetCursor() {
    if (currentChar == '\n')
        return;

    fseek(file, -1, SEEK_CUR);

    if (col == 1) {
        row--;
        col = length_of_previous_row;
    } else {
        col--;
    }
}
