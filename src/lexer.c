#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define strl(str) str, sizeof(str) - 1
#define strsize(str) sizeof(str) - 1

/*
//language could look something like this
function abc {
    x1 = 1 + 2
    x2 = x1 + x2
}

abc()
*/

typedef enum {
    function_keyword, identifier, num_literal, equ, add, sub, open_brace, close_brace, open_parenthesis, close_parenthesis,
} Token_Type;

typedef struct Token {
    Token_Type type;
    char *value;
    int value_size;
    struct Token *next;
} Token;

Token *new_token(Token_Type type, char *value, int value_size) {
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->value_size = value_size;
    token->next = NULL;
    return token;
}

void free_token(Token *token) {
    if (token->type == identifier || token->type == num_literal) {
        free(token->value);
    }
    free(token);
}

void insert_token_list(Token **root, Token *new) {
    if (*root == NULL) {
        *root = new;
    }
    else {
        insert_token_list(&((*root)->next), new);
    }
}

void free_token_list(Token *root) {
    while (root->next != NULL) {
        Token *next = root->next;
        free_token(root);
        root = next;
    }
    free_token(root);
}

int error(char *msg, char *text, int pos) {
    int line = 1;
    int line_pos = 1;
    for (int i = 0; i <= pos; i++) {
        line_pos += 1;
        if (text[i] == '\n') {
            line += 1;
            line_pos = 1;
        }
    }
    printf("ERROR:%d:%d: %s\n", line, line_pos, msg);
    return 1;
}

int cmp(char *first, char *second, int size) {
    return strncmp(first, second, size) == 0;
}

char *skip_past(char *text, char *pattern) {
    text = strstr(text, pattern);
    if (text != NULL) text += 1;
    return text;
}

int is_alphabetical(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int is_numerical(char c) {
    return c >= '0' && c <= '9';
}

int read_num_literal(char *text, int size) {
    int lit_size = 0;
    while (lit_size < size && is_numerical(text[lit_size])) {
        lit_size += 1;
    }
    return lit_size;
}

int read_identifier(char *text, int size) {
    int ident_size = 0;
    if (ident_size < size && is_alphabetical(text[ident_size])) {
        ident_size += 1;
    }
    while (ident_size < size && (is_alphabetical(text[ident_size]) || is_numerical(text[ident_size]))) {
        ident_size += 1;
    }
    return ident_size;
}

int tokenize(char *text, int size, Token **tokens) {
    char *text_start = text;
    int i = 0;
    while (i < size) {

        //skip line comments
        if (cmp(text, strl("//"))) {
            char *before_skip = text;
            text = skip_past(text, "\n");
            if (text == NULL) break;
            i += text - before_skip;
            continue;
        }

        //skip block comments
        if (cmp(text, strl("/*"))) {
            char *before_skip = text;
            text = skip_past(text, "*/");
            if (text == NULL) return error("unclosed block comment", text_start, i);
            i += text - before_skip;
            continue;
        }

        //skip newline
        if (cmp(text, strl("\n"))) {
            text += 1;
            i += 1;
            continue;
        }

        //skip whitespace
        if (cmp(text, strl(" "))) {
            text += 1;
            i += 1;
            continue;
        }

        //function keyword
        if (cmp(text, strl("function"))) {
            text += strsize("function");
            i += strsize("function");
            Token *new = new_token(function_keyword, strl("function"));
            insert_token_list(tokens, new);
            continue;
        }

        //braces
        if (cmp(text, strl("{"))) {
            text += 1;
            i += 1;
            Token *new = new_token(open_brace, strl("{"));
            insert_token_list(tokens, new);
            continue;
        }

        if (cmp(text, strl("}"))) {
            text += 1;
            i += 1;
            Token *new = new_token(close_brace, strl("}"));
            insert_token_list(tokens, new);
            continue;
        }

        if (cmp(text, strl("("))) {
            text += 1;
            i += 1;
            Token *new = new_token(open_parenthesis, strl("("));
            insert_token_list(tokens, new);
            continue;
        }

        if (cmp(text, strl(")"))) {
            text += 1;
            i += 1;
            Token *new = new_token(close_parenthesis, strl(")"));
            insert_token_list(tokens, new);
            continue;
        }

        //operators
        if (cmp(text, strl("="))) {
            text += 1;
            i += 1;
            Token *new = new_token(equ, strl("="));
            insert_token_list(tokens, new);
            continue;
        }

        if (cmp(text, strl("+"))) {
            text += 1;
            i += 1;
            Token *new = new_token(add, strl("+"));
            insert_token_list(tokens, new);
            continue;
        }

        if (cmp(text, strl("-"))) {
            text += 1;
            i += 1;
            Token *new = new_token(sub, strl("-"));
            insert_token_list(tokens, new);
            continue;
        }

        //number literal
        int num_lit_size = read_num_literal(text, size - i);
        if (num_lit_size > 0) {
            text += num_lit_size;
            i += num_lit_size;
            char *new_lit = calloc(num_lit_size + 1, sizeof(char));
            Token *new = new_token(num_literal, new_lit, num_lit_size);
            insert_token_list(tokens, new);
            continue;
        }

        //identifier
        int ident_size = read_identifier(text, size - i);
        if (ident_size > 0) {
            text += ident_size;
            i += ident_size;
            char *new_ident = calloc(ident_size + 1, sizeof(char));
            Token *new = new_token(identifier, new_ident, ident_size);
            insert_token_list(tokens, new);
            continue;
        }

        //end of file (compiler error, loop should have ended before reaching eof)
        if (cmp(text, strl("\0"))) {
            return error("compiler error (lexer)", text_start, i);
        }

        return error("unrecognized character", text_start, i);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("ERROR: please specify input file!\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    char *text = malloc(file_size);
    fread(text, file_size, sizeof(char), file);
    fclose(file);

    Token *tokens = NULL;
    int err = tokenize(text, file_size, &tokens);
    free_token_list(tokens);
    free(text);
    return err;
}
