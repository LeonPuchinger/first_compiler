#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    ND_ROOT, ND_FUNCTION_DEF, ND_FUNCTION_CALL, ND_ASSIGN, ND_INT, ND_VAR, ND_ADD, ND_SUB
} AST_Node_Type;

typedef struct AST_Node {
    AST_Node_Type node_type;
    Token *token;

    //binary AST node
    //used for: assignment, addition, subtraction
    struct AST_Node *lhs, *rhs;

    //n-ary AST node
    //used for: AST root, function definition
    //children: when the node itself has children, next: when the node is part of a list of children
    struct AST_Node *children, *next;

    //no children needed for: function call, integer, variable
} AST_Node;

AST_Node *new_ast_node(Token *token, AST_Node_Type type);

void free_ast_node(AST_Node *node);

void ast_node_add_child(AST_Node *parent, AST_Node *new_child);

#endif
