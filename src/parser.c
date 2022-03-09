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
    new->token = token;
    return new;
}

void free_ast_node(AST_Node *node) {
    free(node);
}

void free_ast_node_list(AST_Node *node) {
    while (node != NULL) {
        AST_Node *next = node->next;
        free_ast_node(node);
        node = next;
    }
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

/* void error(char *msg) {
    printf("ERROR: %msg\n");
    exit(1);
} */

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
    op->lhs->next = op->rhs;
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

//assignment = identifier "=" expression
AST_Node *assignment(Token_List *tokens) {
    //identifier
    Token *id_token = token_list_current(tokens);
    if (id_token == NULL || id_token->type != TK_IDENT) {
        return NULL;
    }
    token_list_forward(tokens);

    //equal sign
    Token *equ_token = token_list_current(tokens);
    if (equ_token == NULL || equ_token->type != TK_ASSIGN) {
        token_list_rewind(tokens, 1);
        return NULL;
    }
    token_list_forward(tokens);

    //expression
    AST_Node *expr = expression(tokens);
    if (expr == NULL) {
        token_list_rewind(tokens, 2);
        return NULL;
    }

    AST_Node *var = new_ast_node(id_token, ND_VAR);
    AST_Node *assign = new_ast_node(equ_token, ND_ASSIGN);
    assign->lhs = var;
    assign->rhs = expr;
    assign->lhs->next = assign->rhs;
    return assign;
}

AST_Node *statement(Token_List *tokens);

//function = "function" identifier "{" { statement } "}"
AST_Node *function(Token_List *tokens) {
    //function keyword
    Token *token = token_list_current(tokens);
    if (token == NULL || token->type != TK_FUNC_KW) {
        return NULL;
    }
    Token_List_Node *token_reset = tokens->current; //return to this token if production can't be matched
    token_list_forward(tokens);

    //identifier
    Token *id_token = token_list_current(tokens);
    if (id_token == NULL || id_token->type != TK_IDENT) {
        token_list_rewind(tokens, 1);
        return NULL;
    }
    token_list_forward(tokens);

    //open brace
    token = token_list_current(tokens);
    if (token == NULL || token->type != TK_OPEN_BRACE) {
        token_list_rewind(tokens, 2);
        return NULL;
    }
    token_list_forward(tokens);

    //statements
    AST_Node *statements = NULL, *current_statement = NULL, *new_statement = NULL;
    while ((new_statement = statement(tokens)) != NULL) {
        if (statements == NULL) {
            statements = new_statement;
            current_statement = new_statement;
        }
        else {
            current_statement->next = new_statement;
            current_statement = new_statement;
        }
    }

    //close brace
    token = token_list_current(tokens);
    if (token == NULL || token->type != TK_CLOSE_BRACE) {
        tokens->current = token_reset; //cannot rewind a known distance because statement count is unknown at compile time
        free_ast_node_list(statements);
        return NULL;
    }
    token_list_forward(tokens);

    AST_Node *function = new_ast_node(id_token, ND_FUNCTION_DEF);
    function->children = statements;
    return function;
}

//boolean = summand "==" summand | summand "!=" summand
AST_Node *boolean(Token_List *tokens) {
    //return to this token if production can't be matched
    Token_List_Node *token_reset = tokens->current;

    //first summand
    AST_Node *s1 = summand(tokens);
    if (s1 == NULL) return NULL;

    //operand
    Token *token = token_list_current(tokens);
    AST_Node *op;
    if (token == NULL) {
        tokens->current = token_reset;
        free_ast_node(s1);
        return NULL;
    }
    if (token->type == TK_EQU) {
        op = new_ast_node(token, ND_BOOLEAN);
    }
    else if (token->type == TK_NON_EQU) {
        op = new_ast_node(token, ND_BOOLEAN);
    }
    else {
        tokens->current = token_reset;
        free_ast_node(s1);
        return NULL;
    }

    token_list_forward(tokens);

    //second summand
    AST_Node *s2 = summand(tokens);
    if (s2 == NULL) {
        tokens->current = token_reset;
        free_ast_node(s1);
        free_ast_node(op);
        return NULL;
    }
    op->lhs = s1;
    op->rhs = s2;
    op->lhs->next = op->rhs;
    return op;
}

//statement = assignment | call | function
AST_Node *statement(Token_List *tokens) {
    AST_Node *node = assignment(tokens);
    if (node != NULL) {
        return node;
    }

    node = call(tokens);
    if (node != NULL) {
        return node;
    }

    return function(tokens);
}

//S = { statement }
AST_Node *parse(Token_List *tokens) {
    //statements
    AST_Node *statements = NULL, *current_statement = NULL, *new_statement = NULL;
    while (token_list_current(tokens) != NULL && (new_statement = statement(tokens)) != NULL) {
        if (statements == NULL) {
            statements = new_statement;
            current_statement = new_statement;
        }
        else {
            current_statement->next = new_statement;
            current_statement = new_statement;
        }
    }

    AST_Node *root = new_ast_node(NULL, ND_ROOT);
    root->children = statements;
    return root;
}
