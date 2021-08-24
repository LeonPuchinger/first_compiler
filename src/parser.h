#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct PT_Node {
    struct PT_Node *children;
    struct PT_Node *next;
    Token *token;
} PT_Node;

#endif
