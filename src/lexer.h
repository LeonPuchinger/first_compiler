#ifndef LEXER_H
#define LEXER_H

typedef enum {
    function_keyword, identifier, num_literal, equ, add, sub, open_brace, close_brace, open_parenthesis, close_parenthesis,
} Token_Type;

typedef struct Token {
    Token_Type type;
    char *value;
    int value_size;
} Token;

Token *new_token(Token_Type type, char *value, int value_size);

void free_token(Token *token);

typedef struct Token_List_Node {
    struct Token_List_Node *next, *previous;
    Token *token;
} Token_List_Node;

Token_List_Node *new_token_list_node(Token *token);

void free_token_list_node(Token_List_Node *node);

typedef struct {
    Token_List_Node *root, *current, *last;
} Token_List;

Token_List *new_token_list();

void free_token_list(Token_List *list);

void token_list_add(Token_List *list, Token *new);

Token *token_list_current(Token_List *list);

Token *token_list_next(Token_List *list);

void token_list_forward(Token_List *list);

void token_list_rewind(Token_List *list, int distance);

int tokenize(char *text, int size, Token_List *tokens);

#endif
