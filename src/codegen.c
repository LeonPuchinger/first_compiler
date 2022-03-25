#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include "codegen.h"
#include "parser.h"
#include "symbol.h"

#define INDENT_WIDTH 4
#define REGISTER_SIZE 8 //64-bit ^= 8 byte
#define FUNC_BUFFERS_PATH "out/func_buffers"

//amount of bytes on the stack in addition to vars (e.g. return addrs) in bytes
static int current_stack_addr_offset = 0;
//every symbol that is represented by a label in the gererated code gets this index appended to make it unique
static int current_mangle_index = 0;

//varargs style version of writef
void vwritef(FILE *file, int indent_enabled, char *fmt, va_list fmt_args) {
    //indent line if requested
    if (indent_enabled) {
        for (int i = 0; i < INDENT_WIDTH; i++) {
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

char *comb_str(char *first, char *second) {
    int size = strlen(first) + strlen(second) + 1;
    char *output = calloc(size, sizeof(char));
    strcpy(output, first);
    return strcat(output, second);
}

void write_header(FILE *file) {
    char header[] =
        "section .text\n"
        "global _start\n\n"
        "_start:";
    writelnf_ni(file, header);
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
        //assign memory address to actual vars, but also
        //assign memory address to functions (to store stack pointer at the beginning of the function)
        current_symbol->addr = addr_offset;
        addr_offset += 1;
        //also set mangle index for function symbols while we're at it
        if (current_symbol->type == SYM_FUNC) {
            current_symbol->mangle_index = current_mangle_index;
            current_mangle_index += 1;
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

int write_function_def(AST_Node *function_def, Symbol_Table *table) {
    FILE *out_file = fopen(comb_str(comb_str(FUNC_BUFFERS_PATH, "/"), function_def->token->value), "w+");

    writelnf_ni(out_file, "%s_%d:", function_def->token->value, symbol_table_get(table, function_def->token)->mangle_index);
    current_stack_addr_offset += 1;
    if (function_def->children == NULL) {
        writelnf(out_file, "nop");
    }
    else {
        writelnf(out_file, "push rsp");
        int err = write_statements(function_def->children, table, out_file);
        if (err) return 1;
        int addr = stack_addr(symbol_table_get(table, function_def->token)->addr);
        writelnf(out_file, "mov rsp, [rbp - %d]", addr);
    }
    writelnf(out_file, "ret\n");
    current_stack_addr_offset -= 1;
    fclose(out_file);
    return 0;
}

void write_function_call(AST_Node *function_call, Symbol_Table *table, FILE *out_file) {
    writelnf(out_file, "call %s_%d\n", function_call->token->value, symbol_table_get(table, function_call->token)->mangle_index);
}

void write_boolean(AST_Node *boolean, Symbol_Table *table, FILE *out_file) {
    AST_Node *lhs = boolean->lhs;
    AST_Node *rhs = boolean->rhs;
    if (lhs->node_type == ND_INT && rhs->node_type == ND_INT) {
        char *constant1 = lhs->token->value;
        writelnf(out_file, "mov rax, %s", constant1);
        char *constant2 = rhs->token->value;
        writelnf(out_file, "mov rbx, %s", constant2);
        writelnf(out_file, "cmp rax, rbx");
    }
    else if (lhs->node_type == ND_VAR && rhs->node_type == ND_INT) {
        char *constant = rhs->token->value;
        writelnf(out_file, "mov rax, %s", constant);
        int addr = stack_addr(symbol_table_get(table, lhs->token)->addr);
        writelnf(out_file, "cmp [rbp - %d], rax", addr);
    }
    else if (lhs->node_type == ND_INT && rhs->node_type == ND_VAR) {
        char *constant = lhs->token->value;
        writelnf(out_file, "mov rax, %s", constant);
        int addr = stack_addr(symbol_table_get(table, rhs->token)->addr);
        writelnf(out_file, "cmp [rbp - %d], rax", addr);
    }
    else {
        int addr1 = stack_addr(symbol_table_get(table, lhs->token)->addr);
        int addr2 = stack_addr(symbol_table_get(table, rhs->token)->addr);
        writelnf(out_file, "mov rax, [rbp - %d]", addr1);
        writelnf(out_file, "cmp rax, [rbp - %d]", addr2);
    }
}

void write_condition(AST_Node *condition, Symbol_Table *table, FILE *out_file, int *scope_index) {
    write_boolean(condition->ms, table, out_file);
    if (condition->ms->token->type == TK_EQU) {
        writef(out_file, "jne ");
    }
    else {
        writef(out_file, "je ");
    }
    if (condition->rhs != NULL) {
        //'else case' exits
        writelnf_ni(out_file, "else_%d", current_mangle_index);
    }
    else {
        writelnf_ni(out_file, "end_%d", current_mangle_index);
    }
    writef(out_file, "\n");

    //set correct scope
    symbol_table_walk_child(table);
    for (int i = 0; i < *scope_index; i++) {
        symbol_table_walk_next(table);
    }
    *scope_index += 1;

    //write statements of 'true-case'
    write_statements(condition->lhs->children, table, out_file);

    if (condition->rhs != NULL) {
        writelnf(out_file, "jmp end_%d", current_mangle_index);

        symbol_table_walk_next(table);
        *scope_index += 1;

        writef(out_file, "\n");
        writelnf(out_file, "else_%d:\n", current_mangle_index);
        //write statements of 'false-case'
        write_statements(condition->rhs->children, table, out_file);
    }
    writelnf(out_file, "end_%d:\n", current_mangle_index);

    symbol_table_pop(table);

    current_mangle_index += 1;
}

int write_statements(AST_Node *statements, Symbol_Table *table, FILE *out_file) {
    //used to keep track of which symbol table child scope is needed when writing function defs or conditions
    int scope_index = 0;
    AST_Node *current_statement = statements;
    while (current_statement != NULL) {
        if (current_statement->node_type == ND_ASSIGN) {
            int err = write_assign(current_statement, table, out_file);
            if (err) return 1;
        }
        else if (current_statement->node_type == ND_FUNCTION_DEF) {
            symbol_table_walk_child(table);
            for (int i = 0; i < scope_index; i++) {
                symbol_table_walk_next(table);
            }
            int err = write_function_def(current_statement, table);
            if (err) return 1;
            symbol_table_pop(table);
            scope_index += 1;
        }
        else if (current_statement->node_type == ND_FUNCTION_CALL) {
            write_function_call(current_statement, table, out_file);
        }
        else if (current_statement->node_type == ND_COND) {
            //need to pass scope_index into the function, because the child scope needs to be set after the boolean is analyzed
            //and the scope needs to change one additional time if the condition has an 'else case'
            write_condition(current_statement, table, out_file, &scope_index);
        }
        else {
            printf("ERROR: AST_Node is not a statement\n");
            return 1;
        }
        current_statement = current_statement->next;
    }

    return 0;
}

//merge output of function definition buffer files back into 'main' output file
int merge_func_buffers(FILE *out_file) {
    DIR *dir = opendir(FUNC_BUFFERS_PATH);
    struct dirent *dirent;
    if (dir == NULL) {
        printf("ERROR: could not list contents of temporary output directory\n");
        return 1;
    }
    //list all files in func buffer dir
    fprintf(out_file, "\n");
    while ((dirent = readdir(dir)) != NULL) {
        //check if actually a file
        if (dirent->d_type == DT_REG) {
            //merge into 'main' output file
            char *func_buffer_file_path = comb_str(comb_str(FUNC_BUFFERS_PATH, "/"), dirent->d_name);
            FILE *func_buffer_file = fopen(func_buffer_file_path, "r");
            fseek(func_buffer_file, 0, SEEK_END);
            int file_size = ftell(func_buffer_file);
            if (file_size > 0) {
                rewind(func_buffer_file);
                char *function_buffer = calloc(file_size + 1, sizeof(char));
                fread(function_buffer, file_size, sizeof(char), func_buffer_file);
                fprintf(out_file, "%s", function_buffer);
            }
            fclose(func_buffer_file);
            int err = remove(func_buffer_file_path);
            if (err) {
                printf("WARNING: could not remove temporary output file %s\n", func_buffer_file_path);
            }
        }
    }
    closedir(dir);
    int err = remove(FUNC_BUFFERS_PATH);
    if (err) {
        printf("WARNING: could not remmove temporary output directory %s\n", FUNC_BUFFERS_PATH);
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

    struct stat st = { 0 };
    if (stat(FUNC_BUFFERS_PATH, &st) == -1) {
        if (mkdir(FUNC_BUFFERS_PATH, 0777) == -1) {
            printf("ERROR: could not create temporary output directory\n");
            return 1;
        }
    }

    int err = write_statements(ast_root->children, table, out_file);
    if (err) return err;

    write_exit(out_file);

    err = merge_func_buffers(out_file);
    if (err) return err;

    return 0;
}
