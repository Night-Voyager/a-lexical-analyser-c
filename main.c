#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

char token[100] = {'\0'};
int token_length = 0;

int isKeyword(char *);
int isOperator(char);
int isPreprocessorDirective(char *);
void handleComments(FILE *);
void handlePunctuations(FILE *, char);
void handleConstants(FILE *, char);
void handleKeywordsAndIdentifiers(FILE *, char);

int main() {
    FILE * file = fopen("../test.c", "r");
    char c;

    while ( (c = getc(file)) != EOF ) {
        if (!isspace(c)) {
            // handle comments
            if (c == '/') {
                handleComments(file);
                continue;
            }

            // initialize token buffer
            token[0] = '\0';
            token_length = 0;

            // handle constants
            if (isdigit(c) || c == '\'' || c == '\"') {
                handleConstants(file, c);
                continue;
            }

            // handle keywords and identifiers
            if (isalpha(c) || c == '_') {
                handleKeywordsAndIdentifiers(file, c);
                continue;
            }

            // handle punctuations
            if (ispunct(c)) {
                handlePunctuations(file, c);
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
            // TODO: handle sign of division
            printf("error: expected expression before '%c' token\n", c);
    }
}

void handleConstants(FILE * file, char c) {
    switch (c) {
        case '\'':
            // handle single character
            do {
                token[token_length++] = c;
                if (c == '\\')
                    token[token_length++] = getc(file);
            } while ( (c = getc(file)) != '\'' && c != '\r' && c != '\n');

            token[token_length++] = c;
            token[token_length] = '\0';

            // check for the terminating ' character
            if (c != '\'') {
                printf("error: missing terminating ' character\n");
                return;
            }

            // check for the length of the token
            if ( (token[1] == '\\' && token_length > 4) || (token[1] != '\\' && token_length > 3) ) {
                printf("warning: multi-character character constant\n");
                return;
            }

            printf("<char, %s>\n", token);

            break;
        case '\"':
            // handle string
            do {
                token[token_length++] = c;
                if (c == '\\')
                    token[token_length++] = getc(file);
            } while ( (c = getc(file)) != '\"' && c != '\r' && c != '\n');

            token[token_length++] = c;
            token[token_length] = '\0';
            token_length = 0;

            if (c == '\r' || c == '\n') {  // handle error
                printf("error: missing terminating \" character\n");
                return;
            }

            printf("<string, %s>\n", token);

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
            fseek(file, -1, SEEK_CUR);

            printf("<num, %s>\n", token);
    }
}

void handlePunctuations(FILE * file, char c){
    if (isOperator(c)) {
        switch (c) {
            case '+':
            case '-':
            {
                char c_next = getc(file);
                if (isdigit(c_next)) {
                    token[token_length++] = c;
                    handleConstants(file, c_next);
                    return;
                }
            }
            default:
                printf("<op, %c>\n", c);
        }
    }
    else {
        switch (c) {
            case '#':
                do {
                    token[token_length++] = c;
                } while (isalpha(c = getc(file)) && token_length < IDENTIFIER_MAX_LEN);

                if (token_length == IDENTIFIER_MAX_LEN) token_length--;
                token[token_length] = '\0';
                token_length = 0;

                if (isPreprocessorDirective(token))
                    printf("<preprocessor directive, %s>\n", token);
                else
                    printf("error: invalid preprocessing directive %s\n", token);

                break;
            default:
                printf("<special symbol, %c>\n", c);
        }
    }

    token_length = 0;
}

void handleKeywordsAndIdentifiers(FILE * file, char c) {
    while ( (isalnum(c) || c == '_') && token_length < IDENTIFIER_MAX_LEN ) {
        token[token_length++] = c;
        c = getc(file);
    }
    token[token_length] = '\0';
    token_length = 0;
    fseek(file, -1, SEEK_CUR);
    if (isKeyword(token)) {
        if (strcmp(token, "sizeof") == 0)
            printf("<op, sizeof>\n");
        else
            printf("<keyword, %s>\n", token);
    }
    else
        printf("<id, %s>\n", token);
}
