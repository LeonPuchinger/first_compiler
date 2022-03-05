#include <stdio.h>
#include <stdarg.h>
#include "codegen.h"
#include "parser.h"
#include "symbol.h"

#define INDENT_WIDTH 4
#define REGISTER_SIZE 8 //64-bit ^= 8 byte

//indentation level for generated code
static int current_indent = 0;
//amount of bytes on the stack in addition to vars (e.g. return addrs) in bytes
static int current_stack_addr_offset = 0;

//varargs style version of writef
void vwritef(FILE *file, int indent, char *fmt, va_list fmt_args) {
    //indent line if requested
    if (indent) {
        for (int i = 0; i < current_indent * INDENT_WIDTH; i++) {
            fprintf(file, " ");
        }
    }
    vfprintf(file, fmt, fmt_args);
}

//write indented output to file with variable amount of format parameters
void writef(FILE *file, char *fmt, ...) {
    va_list fmt_args;
    va_start(fmt_args, fmt);
    vwritef(file, 1, fmt, fmt_args);
    va_end(fmt_args);
}

//like writef, but do not indent
void writef_ni(FILE *file, char *fmt, ...) {
    va_list fmt_args;
    va_start(fmt_args, fmt);
    vwritef(file, 0, fmt, fmt_args);
    va_end(fmt_args);
}

//write indented output to file with newline and variable amount of format parameters
void writelnf(FILE *file, char *fmt, ...) {
    va_list fmt_args;
    va_start(fmt_args, fmt);
    vwritef(file, 1, fmt, fmt_args);
    va_end(fmt_args);
    fprintf(file, "\n");
}

//like writelnf, but do not indent
void writelnf_ni(FILE *file, char *fmt, ...) {
    va_list fmt_args;
    va_start(fmt_args, fmt);
    vwritef(file, 0, fmt, fmt_args);
    va_end(fmt_args);
    fprintf(file, "\n");
}

void write_header(FILE *file) {
    char header[] =
        "section .text\n"
        "global _start\n\n"
        "_start:";
    writelnf(file, header);
    current_indent += 1;
    writelnf(file, "mov rbp, rsp\n");
}

void write_exit(FILE *file) {
    writelnf(file, "mov rax, 60");
    writelnf(file, "mov rdi, 0");
    writelnf(file, "syscall");
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
    return (current_stack_addr_offset + 1 + virtual_addr) * REGISTER_SIZE;
}

int write_statements(AST_Node *statements, Symbol_Table *table, FILE *out_file);

int write_assign(AST_Node *assignment, Symbol_Table *table, FILE *out_file) {
    AST_Node *assignee = assignment->lhs;
    AST_Node *expr = assignment->rhs;
    Symbol *assignee_sym = symbol_table_is_local(table, assignee->token);
    int is_initial_assign = assignee_sym != NULL && !assignee_sym->initialized;
    if (is_initial_assign) {
        assignee_sym->initialized = 1;
    }
    int is_composite = expr->node_type == ND_ADD || expr->node_type == ND_SUB;
    if (is_initial_assign) {
        if (is_composite) {
            //init = a +/- b
            writef(out_file, "mov rax, ");
            if (expr->lhs->node_type == ND_VAR) {
                //init = var +/- ...
                int addr = stack_addr(symbol_table_get(table, expr->lhs->token)->addr);
                writelnf_ni(out_file, "[rbp - %d]", addr);
            }
            else {
                //init = const +/- ...
                char *constant = expr->lhs->token->value;
                writelnf_ni(out_file, "%s", constant);
            }
            //write operator
            if (expr->node_type == ND_ADD) {
                writef(out_file, "add rax, ");
            }
            else {
                writef(out_file, "sub rax, ");
            }
            if (expr->rhs->node_type == ND_VAR) {
                //init = ... +/- var
                int addr = stack_addr(symbol_table_get(table, expr->rhs->token)->addr);
                writelnf_ni(out_file, "[rbp - %d]", addr);
            }
            else {
                //init = ... +/- const
                char *constant = expr->rhs->token->value;
                writelnf_ni(out_file, "%s", constant);
            }
            writelnf(out_file, "push rax");
        }
        else {
            //init = var/const
            writef(out_file, "push ");
            if (expr->node_type == ND_VAR) {
                //init = var
                int addr = stack_addr(symbol_table_get(table, expr->token)->addr);
                writelnf_ni(out_file, "[rbp - %d]", addr);
            }
            else {
                //init = const
                char *constant = expr->token->value;
                writelnf_ni(out_file, "%s", constant);
            }
        }
    }
    else {
        int assignee_addr = stack_addr(symbol_table_get(table, assignee->token)->addr);
        if (is_composite) {
            //exist = a +/- b
            if (expr->lhs->node_type == ND_INT && expr->rhs->node_type == ND_INT) {
                //exist = const +/- const
                char *constant1 = expr->lhs->token->value;
                writelnf(out_file, "mov [rbp - %d], %s", assignee_addr, constant1);
                //write operator
                if (expr->node_type == ND_ADD) {
                    writef(out_file, "add ");
                }
                else {
                    writef(out_file, "sub ");
                }
                char *constant2 = expr->rhs->token->value;
                writelnf_ni(out_file, "[rbp - %d], %s", assignee_addr, constant2);
            }
            else {
                //exist = var +/- var | const +/- var | var +/- const
                writef(out_file, "mov rax, ");
                if (expr->lhs->node_type == ND_VAR) {
                    //exist = var +/- ...
                    int addr = stack_addr(symbol_table_get(table, expr->lhs->token)->addr);
                    writelnf_ni(out_file, "[rbp - %d]", addr);
                }
                else {
                    //exist = const +/- ...
                    char *constant = expr->lhs->token->value;
                    writelnf_ni(out_file, "%s", constant);
                }
                //write operator
                if (expr->node_type == ND_ADD) {
                    writef(out_file, "add ");
                }
                else {
                    writef(out_file, "sub ");
                }
                writef_ni(out_file, "rax, ");
                if (expr->rhs->node_type == ND_VAR) {
                    //exist = ... +/- var
                    int addr = stack_addr(symbol_table_get(table, expr->rhs->token)->addr);
                    writelnf_ni(out_file, "[rbp - %d]", addr);
                }
                else {
                    //exist = ... +/- const
                    char *constant = expr->rhs->token->value;
                    writelnf_ni(out_file, "%s", constant);
                }
                //store result
                writelnf(out_file, "mov [rbp - %d], rax", assignee_addr);
            }
        }
        else {
            //exist = var/const
            if (expr->node_type == ND_VAR) {
                //exist = var
                int addr = stack_addr(symbol_table_get(table, expr->token)->addr);
                writelnf(out_file, "mov rax, [rbp - %d]", addr);
                writelnf(out_file, "mov [rbp - %d], rax", assignee_addr);
            }
            else {
                //exist = const
                char *constant = expr->token->value;
                writelnf(out_file, "mov qword [rbp - %d], %s", assignee_addr, constant);
            }
        }
    }
    writef(out_file, "\n");
    return 0;
}

int write_function_def(AST_Node *function_def, Symbol_Table *table, FILE *out_file) {
    writelnf(out_file, "%s:", function_def->token->value);
    current_indent += 1;
    current_stack_addr_offset += 1;
    if (function_def->children == NULL) {
        writelnf(out_file, "nop");
    } else {
        int err = write_statements(function_def->children, table, out_file);
        if (err) return 1;
    }
    writelnf(out_file, "ret\n");
    current_indent -= 1;
    current_stack_addr_offset -= 1;
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
            int err = write_function_def(current_statement, table, out_file);
            if (err) return 1;
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

    int err = write_statements(ast_root->children, table, out_file);
    if (err) return err;

    write_exit(out_file);
    return 0;
}
