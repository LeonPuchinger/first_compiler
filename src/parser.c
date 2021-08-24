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
    Token *token = token_list_next(tokens);
    if (token->type == identifier || token->type == num_literal) {
        PT_Node *new = new_pt_node(token);
        return new;
    }
    //TODO: maybe implement token_list_current for simple productions like this, only forward if actually needed
    token_list_rewind(tokens, 1);
    return NULL;
}
