#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct PT_Node {
    struct PT_Node *children;
    struct PT_Node *next;
    Token *token;
} PT_Node;

PT_Node *new_pt_node(Token *token);

void free_pt_node(PT_Node *node);

void pt_node_add_child(PT_Node *parent, PT_Node *new);

#endif
