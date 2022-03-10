#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    ND_ROOT, ND_FUNCTION_DEF, ND_FUNCTION_CALL, ND_BOOLEAN, ND_ASSIGN, ND_INT, ND_VAR, ND_ADD, ND_SUB
} AST_Node_Type;

typedef struct AST_Node {
    AST_Node_Type node_type;
    Token *token;

    //binary AST node
    //used for: assignment, boolean, addition, subtraction
    struct AST_Node *lhs, *rhs;
    //tertiary AST Node (in addition to lhs, rhs)
    //used for: condition
    struct AST_Node *ms;

    //n-ary AST node
    //used for: AST root, function definition, true condition, false condition
    //children: when the node itself has children
    struct AST_Node *children;

    //next: when the node is part of a list of children
    //important: also use next in binary nodes to make iteration easier (lhs->next == rhs, rhs->next == NULL)
    struct AST_Node *next;

    //no children needed for: function call, integer, variable
} AST_Node;

AST_Node *new_ast_node(Token *token, AST_Node_Type type);

void free_ast_node(AST_Node *node);

void free_ast_node_recursive(AST_Node *node);

void free_ast_node_list(AST_Node *node);

void free_ast_node_list_recursive(AST_Node *node);

void ast_node_add_child(AST_Node *parent, AST_Node *new_child);

AST_Node *parse(Token_List *tokens);

#endif
