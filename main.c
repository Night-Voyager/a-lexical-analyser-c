#include <stdio.h>
#include <string.h>
#include <ctype.h>

static char keywords[][11] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float",
    "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", "struct", "switch",
    "typeof", "union", "unsigned", "void", "volatile", "while",  // 32 keywords for ANSI C
    "_Bool", "_Complex", "_Imaginary", "inline", "restrict"  // 5 new keywords for C99
};

int isKeyword(char *);

int main() {
    char token[32] = {'\0'};
    int token_length = 0;

    FILE * file = fopen("../main.c", "r");
    char c;

    while ( (c = getc(file)) != EOF ) {
        if (isspace(c) == 0 && token_length < 32) {
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
