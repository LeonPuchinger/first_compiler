#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "symbol.h"

int check_symbols(AST_Node *ast_root, Symbol_Table *table) {
    AST_Node *statement;
    //check if root node has children in form of list or lhs/rhs
    if (ast_root->node_type == ND_ROOT || ast_root->node_type == ND_FUNCTION_DEF) {
        statement = ast_root->children;
    } else {
        statement = ast_root->lhs;
        //TODO add mechanism to signal if rhs or NULL is next during forward
    }

    int err;
    while (statement != NULL) {
        if (statement->node_type == ND_FUNCTION_DEF) {
            //check and register function name
            if (symbol_table_get(table, statement->token)) {
                printf("ERROR: redefinition of %s\n", statement->token->value);
                return 1;
            }
            symbol_table_set(table, new_symbol(SYM_FUNC, statement->token));
            //check function contents
            symbol_table_push(table);
            err = check_symbols(statement->children, table);
            if (err) return err;
            symbol_table_pop(table);
            //forward
            statement = statement->next;
        }
        else if (statement->node_type == ND_FUNCTION_CALL) {
            Symbol *sym = symbol_table_get(table, statement->token);
            //check if function is defined
            if (!sym) {
                printf("ERROR: %s is not defined\n", statement->token->value);
                return 1;
            }
            //check if function even is a function
            if (sym->type != SYM_FUNC) {
                printf("ERROR: %s is not callable\n", statement->token->value);
                return 1;
            }
            statement = statement->next;
        }
        else if (statement->node_type == ND_ASSIGN) {
            //register assigned symbol if it does not exist yet
            if (!symbol_table_get(table, statement->lhs->token)) {
                symbol_table_set(table, new_symbol(SYM_INT, statement->lhs->token));
            }
            //check expression symbols
            err = check_symbols(statement->rhs, table);
            if (err) return err;

            statement = statement->next;
        }
        else if (statement->node_type == ND_ADD || statement->node_type == ND_SUB) {
            //check both parts of addition or subtraction
            err = check_symbols(statement->lhs, table);
            if (err) return err;
            err = check_symbols(statement->rhs, table);
            if (err) return err;
        }
        //TODO add handlers for missing node types
        else {
            printf("INTERNAL ERROR: cannot process AST-Node Type\n");
            return 1;
        }
    }

    return 0;
}

int semantic_analysis(AST_Node *ast_root, Symbol_Table *table) {
    if (ast_root->node_type != ND_ROOT) {
        printf("INTERNAL ERROR: AST has no root node\n");
        return 1;
    }

    return check_symbols(ast_root, table);
}
