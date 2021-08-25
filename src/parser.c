#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

PT_Node *new_pt_node(Token *token) {
    PT_Node *new = malloc(sizeof(PT_Node));
    new->children = NULL;
    new->next = NULL;
    new->token = token;
    return new;
}

void free_pt_node(PT_Node *node) {
    free(node);
}

void pt_node_add_child(PT_Node *parent, PT_Node *new) {
    if (parent->children == NULL) {
        parent->children = new;
    }
    else {
        PT_Node *children = parent->children;
        while (children->next != NULL) {
            children = children->next;
        }
        children->next = new;
    }
}

void error(char *msg) {
    printf("ERROR: %msg\n");
    exit(1);
}

//summand = ident | num_literal
PT_Node *summand(Token_List *tokens) {
    Token *token = token_list_current(tokens);
    if (token == NULL) return NULL;
    if (token->type == identifier || token->type == num_literal) {
        PT_Node *new = new_pt_node(token);
        token_list_forward(tokens);
        return new;
    }
    return NULL;
}

//expression = summand "+" summand | summand "-" summand | summand
PT_Node *expression(Token_List *tokens) {
    //all alternatives have first summand in common
    PT_Node *s1 = summand(tokens);
    if (s1 == NULL) return NULL;

    PT_Node *new = new_pt_node(NULL); //TODO: add ability to add node kind/type (e.g. expr)
    pt_node_add_child(new, s1);

    //operands
    Token *token = token_list_current(tokens);
    PT_Node *op;
    if (token == NULL) return new;
    if (token->type == add || token->type == sub) {
        op = new_pt_node(token);
    }
    else {
        //no operand found, but single summand is still a valid expression
        return new;
    }
    token_list_forward(tokens);

    //second summand
    PT_Node *s2 = summand(tokens);
    if (s2 == NULL) {
        //rewind already consumed operator and free its PT node
        token_list_rewind(tokens, 1);
        free_pt_node(op);
        return new;
    }
    pt_node_add_child(new, op);
    pt_node_add_child(new, s2);
    return new;
}

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
}
