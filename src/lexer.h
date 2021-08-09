#ifndef LEXER_H
#define LEXER_H

typedef enum {
    function_keyword, identifier, num_literal, equ, add, sub, open_brace, close_brace, open_parenthesis, close_parenthesis,
} Token_Type;

typedef struct Token {
    Token_Type type;
    char *value;
    int value_size;
    struct Token *next;
} Token;

Token *new_token(Token_Type type, char *value, int value_size);

void free_token(Token *token);

int tokenize(char *text, int size, Token **tokens);

#endif
