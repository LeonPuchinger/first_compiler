#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

#define strl(str) str, sizeof(str) - 1
#define strsize(str) sizeof(str) - 1

#define BACKLOG_MAX_SIZE 5

Token *new_token(Token_Type type, char *value, int value_size) {
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->value_size = value_size;
    return token;
}

void free_token(Token *token) {
    if (token->type == identifier || token->type == num_literal) {
        free(token->value);
    }
    free(token);
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

//Token List Node

Token_List_Node *new_token_list_node(Token *token) {
    Token_List_Node *new = malloc(sizeof(Token_List_Node));
    new->next = NULL;
    new->previous = NULL;
    new->token = token;
    return new;
}

void free_token_list_node(Token_List_Node *node) {
    free(node);
}

//Token List

Token_List *new_token_list() {
    Token_List *new = malloc(sizeof(Token_List));
    new->root = NULL;
    new->current = NULL;
    new->last = NULL;
    return new;
}

void free_token_list(Token_List *list) {
    Token_List_Node *node = list->root;
    while (node != NULL) {
        Token_List_Node *next = node->next;
        free_token(node->token);
        free_token_list_node(node);
        node = next;
    }
    free(list);
}

void token_list_add(Token_List *list, Token *new) {
    Token_List_Node *new_node = new_token_list_node(new);
    if (list->root == NULL) {
        list->root = new_node;
        list->current = new_node;
        list->last = new_node;
    }
    else {
        if (list->last != NULL) {
            new_node->previous = list->last;
            list->last->next = new_node;
            list->last = new_node;
        }
    }
}

Token *token_list_next(Token_List *list) {
    if (list->root == NULL || list->current == NULL) {
        return NULL;
    }
    Token *token = list->current->token;
    list->current = list->current->next;
    return token;
}

void token_list_rewind(Token_List *list, int distance) {
    if (list->root != NULL && list->last != NULL) {
        while (distance > 0) {
            if (list->current == list->root || list->current->previous == NULL) {
                return;
            }
            if (list->current == NULL) {
                list->current = list->last;
            }
            else {
                list->current = list->current->previous;
            }
            distance -= 1;
        }
    }
}

int tokenize(char *text, int size, Token_List *tokens) {
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
            token_list_add(tokens, new);
            continue;
        }

        //braces
        if (cmp(text, strl("{"))) {
            text += 1;
            i += 1;
            Token *new = new_token(open_brace, strl("{"));
            token_list_add(tokens, new);
            continue;
        }

        if (cmp(text, strl("}"))) {
            text += 1;
            i += 1;
            Token *new = new_token(close_brace, strl("}"));
            token_list_add(tokens, new);
            continue;
        }

        if (cmp(text, strl("("))) {
            text += 1;
            i += 1;
            Token *new = new_token(open_parenthesis, strl("("));
            token_list_add(tokens, new);
            continue;
        }

        if (cmp(text, strl(")"))) {
            text += 1;
            i += 1;
            Token *new = new_token(close_parenthesis, strl(")"));
            token_list_add(tokens, new);
            continue;
        }

        //operators
        if (cmp(text, strl("="))) {
            text += 1;
            i += 1;
            Token *new = new_token(equ, strl("="));
            token_list_add(tokens, new);
            continue;
        }

        if (cmp(text, strl("+"))) {
            text += 1;
            i += 1;
            Token *new = new_token(add, strl("+"));
            token_list_add(tokens, new);
            continue;
        }

        if (cmp(text, strl("-"))) {
            text += 1;
            i += 1;
            Token *new = new_token(sub, strl("-"));
            token_list_add(tokens, new);
            continue;
        }

        //number literal
        int num_lit_size = read_num_literal(text, size - i);
        if (num_lit_size > 0) {
            i += num_lit_size;
            char *new_lit = calloc(num_lit_size + 1, sizeof(char));
            strncpy(new_lit, text, num_lit_size);
            text += num_lit_size;
            Token *new = new_token(num_literal, new_lit, num_lit_size);
            token_list_add(tokens, new);
            continue;
        }

        //identifier
        int ident_size = read_identifier(text, size - i);
        if (ident_size > 0) {
            i += ident_size;
            char *new_ident = calloc(ident_size + 1, sizeof(char));
            strncpy(new_ident, text, ident_size);
            text += ident_size;
            Token *new = new_token(identifier, new_ident, ident_size);
            token_list_add(tokens, new);
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

    Token_List *list = new_token_list();
    int err = tokenize(text, file_size, list);
    free_token_list(list);
    free(text);
    return err;
}
