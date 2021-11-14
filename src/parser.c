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
    } else {
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
    AST_Node *new;
    if (token->type == identifier) {
        new = new_ast_node(token, ND_VAR);
    }
    else if (token->type == num_literal) {
        new = new_ast_node(token, ND_INT);
    } else {
        return NULL;
    }
    token_list_forward(tokens);
    return new;
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
    if (token->type == add) {
        op = new_ast_node(token, ND_ADD);
    }
    else if (token->type == sub) {
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
/*
//call = identifier "()"
PT_Node *call(Token_List *tokens) {
    //identifier
    Token *token = token_list_current(tokens);
    if (token == NULL || token->type != identifier) {
        return NULL;
    }
    PT_Node *id = new_pt_node(token);
    token_list_forward(tokens);

    //open parenthesis
    token = token_list_current(tokens);
    if (token == NULL || token->type != open_parenthesis) {
        token_list_rewind(tokens, 1);
        free_pt_node(id);
        return NULL;
    }
    PT_Node *open = new_pt_node(token);
    token_list_forward(tokens);

    //close parenthesis
    token = token_list_current(tokens);
    if (token == NULL || token->type != close_parenthesis) {
        token_list_rewind(tokens, 2);
        free_pt_node(id);
        free_pt_node(open);
        return NULL;
    }
    PT_Node *close = new_pt_node(token);
    PT_Node *new = new_pt_node(NULL);
    pt_node_add_child(new, id);
    pt_node_add_child(new, open);
    pt_node_add_child(new, close);
    token_list_forward(tokens);
    return new;
} */
