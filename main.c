#include <stdio.h>
#include <string.h>

static char keywords[][11] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float",
    "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", "struct", "switch",
    "typeof", "union", "unsigned", "void", "volatile", "while",  // 32 keywords for ANSI C
    "_Bool", "_Complex", "_Imaginary", "inline", "restrict"  // 5 new keywords for C99
};

int isKeyword(char *);

int main() {
    FILE * file;
    file = fopen("../main.c", "r");
    char c = getc(file);
    while (c != EOF) {
        putchar(c);
        c = getc(file);
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
