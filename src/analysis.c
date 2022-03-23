#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "symbol.h"

int check_symbols(AST_Node *statement, Symbol_Table *table) {
    int err = 0;
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
        }
        else if (statement->node_type == ND_BOOLEAN) {
            err = check_symbols(statement->lhs, table);
            if (err) return err;
            err = check_symbols(statement->rhs, table);
            if (err) return err;
        }
        else if (statement->node_type == ND_COND) {
            //check condition bool
            err = check_symbols(statement->ms, table);
            if (err) return err;
            //check 'true case' contents
            symbol_table_push(table);
            err = check_symbols(statement->lhs->children, table);
            if (err) return err;
            symbol_table_pop(table);
            //check 'false case' contents
            symbol_table_push(table);
            err = check_symbols(statement->rhs->children, table);
            if (err) return err;
            symbol_table_pop(table);
        }
        else if (statement->node_type == ND_ASSIGN) {
            //rhs of assign needs to be check first
            //this way, a variable can't be assigned to itself during its initial assignment
            err = check_symbols(statement->rhs, table);
            if (err) return err;

            //register assigned symbol if it does not exist yet
            if (!symbol_table_get(table, statement->lhs->token)) {
                symbol_table_set(table, new_symbol(SYM_INT, statement->lhs->token));
            }
            else {
                //if symbol already exists, check it (e.g. for type)
                err = check_symbols(statement->lhs, table);
                if (err) return err;
            }
        }
        else if (statement->node_type == ND_ADD || statement->node_type == ND_SUB) {
            //check both parts of addition or subtraction
            err = check_symbols(statement->lhs, table);
            if (err) return err;
            err = check_symbols(statement->rhs, table);
            if (err) return err;
        }
        else if (statement->node_type == ND_VAR) {
            Symbol *sym = symbol_table_get(table, statement->token);
            if (!sym) {
                printf("ERROR: %s is not defined\n", statement->token->value);
                return 1;
            }
            //check type
            if (sym->type != SYM_INT) {
                printf("ERROR: %s has mismatched type\n", statement->token->value);
                return 1;
            }
        }
        //throw error except for nodes that do not have to be checked
        else if (statement->node_type != ND_INT) {
            printf("INTERNAL ERROR: cannot process AST-Node Type\n");
            return 1;
        }
        //forward
        statement = statement->next;
    }

    return 0;
}

int semantic_analysis(AST_Node *ast_root, Symbol_Table *table) {
    if (ast_root->node_type != ND_ROOT) {
        printf("INTERNAL ERROR: AST has no root node\n");
        return 1;
    }

    return check_symbols(ast_root->children, table);
}
