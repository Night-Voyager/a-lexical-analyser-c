#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define IDENTIFIER_MAX_LEN 32

static char keywords[][11] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float",
    "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", "struct", "switch",
    "typeof", "union", "unsigned", "void", "volatile", "while",  // 32 keywords for ANSI C
    "_Bool", "_Complex", "_Imaginary", "inline", "restrict"  // 5 new keywords for C99
};

char token[100] = {'\0'};
int token_length = 0;

int isKeyword(char *);
void handleComments(FILE *);
void handleConstants(FILE *, char);

int main() {
    FILE * file = fopen("../main.c", "r");
    char c;

    while ( (c = getc(file)) != EOF ) {
        if (isspace(c) == 0 && token_length < 32) {

            // handle comments
            if (c == '/' && token_length == 0) {
                handleComments(file);
                continue;
            }

            // handle constants
            if ( (isdigit(c) || c == '\'' || c == '\"') && token_length == 0 ) {
                handleConstants(file, c);
                continue;
            }

            token[token_length] = c;
            token_length++;
        } else {
            token[token_length] = '\0';

            if (token[0] == '\0') continue;

            if (isKeyword(token) == 0)
                printf("keyword: %s\n", token);
            else
                printf("token: %s\n", token);
            token_length = 0;
        }
    }

    fclose(file);
    return 0;
}

int isKeyword(char * s) {
    int i;
    for (i = 0; i < 37; ++i) {
        if (strcmp(s, keywords[i]) == 0) break;
    }
    return strcmp(s, keywords[i]);
}

void handleComments(FILE * file) {
    char c = getc(file);
    switch (c) {
        case '/':
            while ( (c = getc(file)) != '\n' );
            break;
        case '*':
            while ( (c = getc(file)) != '*' || (c = getc(file)) != '/' );
            break;
        default:
            printf("Error: invalid token");
            exit(0);
    }
}

void handleConstants(FILE * file, char c) {
    switch (c) {
        case '\'':
            // handle single character
            token[token_length++] = c;

            c = getc(file);
            if (c == '\\') {
                token[token_length++] = c;
                c = getc(file);
            }
            token[token_length++] = c;

            if ( (c = getc(file)) != '\'' ) {  // handle error
                printf("warning: multi-character character constant");
                exit(0);
            }

            token[token_length++] = c;
            token[token_length++] = '\0';
            token_length = 0;

            printf("char: %s\n", token);

            break;
        case '\"':
            // handle string
            do {
                token[token_length++] = c;
                if (c == '\\')
                    token[token_length++] = getc(file);
            } while ( (c = getc(file)) != '\"' && c != '\r' && c != '\n');

            if (c == '\r' || c == '\n') {  // handle error
                printf("error: missing terminating \" character");
                exit(0);
            }

            token[token_length++] = c;
            token[token_length] = '\0';
            token_length = 0;

            printf("string: %s\n", token);

            break;
        default:
            // handle number
            while (isdigit(c)) {
                token[token_length++] = c;
                c = getc(file);
            }

            if (c == '.') {
                token[token_length++] = c;
                c = getc(file);
                while (isdigit(c)) {
                    token[token_length++] = c;
                    c = getc(file);
                }
            }

            token[token_length] = '\0';
            token_length = 0;

            printf("num: %s\n", token);
    }
}
