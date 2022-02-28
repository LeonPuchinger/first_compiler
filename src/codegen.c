#include <stdio.h>
#include <stdarg.h>
#include "codegen.h"
#include "parser.h"
#include "symbol.h"

#define INDENT_WIDTH 4
#define REGISTER_SIZE 8 //64-bit ^= 8 byte

static int current_indent = 0;
//amount of bytes on the stack in addition to vars (e.g. return addrs) in bytes
static int current_stack_addr_offset = 0;

//varargs style version of writef
void vwritef(FILE *file, char *fmt, va_list fmt_args) {
    //indent line
    for (int i = 0; i < current_indent * INDENT_WIDTH; i++) {
        fprintf(file, " ");
    }
    vfprintf(file, fmt, fmt_args);
}

//write indented output to file with variable amount of format parameters
void writef(FILE *file, char *fmt, ...) {
    va_list fmt_args;
    va_start(fmt_args, fmt);
    vwritef(file, fmt, fmt_args);
    va_end(fmt_args);
}

//write indented output to file with newline and variable amount of format parameters
void writelnf(FILE *file, char *fmt, ...) {
    va_list fmt_args;
    va_start(fmt_args, fmt);
    vwritef(file, fmt, fmt_args);
    va_end(fmt_args);
    fprintf(file, "\n");
}

void write_header(FILE *file) {
    char header[] =
        "section .text\n"
        "global _start\n\n"
        "_start:";
    writelnf(file, header);
}

void _assign_addrs(Scope *scope, int addr_offset) {
    //assign addr to each symbol
    Collection_Container *current_sym_cont = scope->symbols->root;
    while (current_sym_cont != NULL) {
        Symbol *current_symbol = current_sym_cont->item;
        //only assign addr to actual vars
        if (current_symbol->type != SYM_FUNC) {
            current_symbol->addr = addr_offset;
            addr_offset += 1;
        }
        current_sym_cont = current_sym_cont->next;
    }
    //repeat for every child scope, but start addresses at offset
    Collection_Container *current_scope_cont = scope->scopes->root;
    while (current_scope_cont != NULL) {
        Scope *current_scope = current_scope_cont->item;
        _assign_addrs(current_scope, addr_offset);
        current_scope_cont = current_scope_cont->next;
    }
}

void assign_addrs(Symbol_Table *table) {
    _assign_addrs(table->root_scope, 0);
}

//converts "virtual" symbol-table addr to stack addr, relative to rbp
//relative to rbp means the addr is n bytes lower than rbp
int stack_addr(int virtual_addr) {
    return (current_stack_addr_offset + virtual_addr) * REGISTER_SIZE;
}

int write_assign(AST_Node *assignment, Symbol_Table *table, FILE *out_file) {
    AST_Node *assignee = assignment->lhs;
    AST_Node *expr = assignment->rhs;
    int is_initial_assign = symbol_table_is_local(table, assignee->token);
    int is_composite = expr->node_type == ND_ADD || expr->node_type == ND_SUB;
    if (is_initial_assign) {
        if (is_composite) {
            //init = a +/- b
            writef(out_file, "mov rax ");
            if (expr->lhs->node_type == ND_VAR) {
                //init = var +/- ...
                int addr = stack_addr(symbol_table_get(table, expr->lhs->token)->addr);
                writelnf(out_file, "rbp - %d", addr);
            }
            else {
                //init = const +/- ...
                char *constant = expr->lhs->token->value;
                writelnf(out_file, "%s", constant);
            }
            //write operator
            if (expr->node_type == ND_ADD) {
                writef(out_file, "add rax ");
            }
            else {
                writef(out_file, "sub rax ");
            }
            if (expr->rhs->node_type == ND_VAR) {
                //init = ... +/- var
                int addr = stack_addr(symbol_table_get(table, expr->rhs->token)->addr);
                writelnf(out_file, "rbp - %d", addr);
            }
            else {
                //init = ... +/- const
                char *constant = expr->rhs->token->value;
                writelnf(out_file, "%s", constant);
            }
            writelnf(out_file, "push rax");
        }
        else {
            //init = var/const
            writef(out_file, "push ");
            if (expr->node_type == ND_VAR) {
                //init = var
                int addr = stack_addr(symbol_table_get(table, expr->token)->addr);
                writelnf(out_file, "rbp - %d", addr);
            }
            else {
                //init = const
                char *constant = expr->token->value;
                writelnf(out_file, "%s", constant);
            }
        }
    }
    else {
        int assignee_addr = stack_addr(symbol_table_get(table, assignee->token)->addr);

    }
    return 0;
}

int write_statements(AST_Node *statements, Symbol_Table *table, FILE *out_file) {
    AST_Node *current_statement = statements;
    while (current_statement != NULL) {
        if (current_statement->node_type == ND_ASSIGN) {
            int err = write_assign(current_statement, table, out_file);
            if (err) return 1;
        }
        else if (current_statement->node_type == ND_FUNCTION_DEF) {
            //TODO write func def
            printf("NOT IMPLEMENTED\n");
            return 1;
        }
        else if (current_statement->node_type == ND_FUNCTION_CALL) {
            //TODO write func call
            printf("NOT IMPLEMENTED\n");
            return 1;
        }
        else {
            printf("ERROR: AST_Node is not a statement\n");
            return 1;
        }

        //TODO "default" -> error: not a statement (assignment call function)

        current_statement = current_statement->next;
    }

    return 0;
}

int codegen(AST_Node *ast_root, Symbol_Table *table, FILE *out_file) {
    write_header(out_file);
    symbol_table_reset_current(table);
    assign_addrs(table);

    if (ast_root->node_type != ND_ROOT) {
        printf("INTERNAL ERROR: AST has no root node\n");
        return 1;
    }

    return write_statements(ast_root->children, table, out_file);
}
