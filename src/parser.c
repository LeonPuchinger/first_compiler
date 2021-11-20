#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

AST_Node *new_ast_node(Token *token, AST_Node_Type type) {
    AST_Node *new = malloc(sizeof(AST_Node));
    new->lhs = NULL;
    new->rhs = NULL;
    new->children = NULL;
    new->next = NULL;
    new->node_type = type;
}

void free_ast_node(AST_Node *node) {
    free(node);
}

void ast_node_add_child(AST_Node *parent, AST_Node *new_child) {
    if (parent->children == NULL) {
        parent->children = new_child;
    }
    else {
        AST_Node *children = parent->children;
        while (children->next != NULL) {
            children = children->next;
        }
        children->next = new_child;
    }
}

void error(char *msg) {
    printf("ERROR: %msg\n");
    exit(1);
}

//summand = ident | num_literal
AST_Node *summand(Token_List *tokens) {
    Token *token = token_list_current(tokens);
    if (token == NULL) return NULL;
    AST_Node *summand;
    if (token->type == TK_IDENT) {
        summand = new_ast_node(token, ND_VAR);
    }
    else if (token->type == TK_NUM_LITERAL) {
        summand = new_ast_node(token, ND_INT);
    }
    else {
        return NULL;
    }
    token_list_forward(tokens);
    return summand;
}

//expression = summand "+" summand | summand "-" summand | summand
AST_Node *expression(Token_List *tokens) {
    //all alternatives have first summand in common
    AST_Node *s1 = summand(tokens);
    if (s1 == NULL) return NULL;

    //operand
    Token *token = token_list_current(tokens);
    AST_Node *op;
    if (token == NULL) {
        return s1;
    }
    if (token->type == TK_ADD) {
        op = new_ast_node(token, ND_ADD);
    }
    else if (token->type == TK_SUB) {
        op = new_ast_node(token, ND_SUB);
    }
    else {
        //no operand found, but single summand is still a valid expression
        return s1;
    }

    token_list_forward(tokens);

    //second summand
    AST_Node *s2 = summand(tokens);
    if (s2 == NULL) {
        //rewind already consumed operator and free its AST node
        token_list_rewind(tokens, 1);
        free_ast_node(op);
        return s1;
    }
    op->lhs = s1;
    op->rhs = s2;
    return op;
}

//call = identifier "()"
AST_Node *call(Token_List *tokens) {
    //identifier
    Token *id_token = token_list_current(tokens);
    if (id_token == NULL || id_token->type != TK_IDENT) {
        return NULL;
    }
    token_list_forward(tokens);

    Token *token;
    //open parenthesis
    token = token_list_current(tokens);
    if (token == NULL || token->type != TK_OPEN_PAREN) {
        token_list_rewind(tokens, 1);
        return NULL;
    }
    token_list_forward(tokens);

    //close parenthesis
    token = token_list_current(tokens);
    if (token == NULL || token->type != TK_CLOSE_PAREN) {
        token_list_rewind(tokens, 2);
        return NULL;
    }
    token_list_forward(tokens);

    return new_ast_node(id_token, ND_FUNCTION_CALL);
}
