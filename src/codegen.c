#include <stdio.h>
#include "codegen.h"
#include "parser.h"
#include "symbol.h"

#define INDENT_WIDTH 4

static int current_indent = 0;

void write(FILE *file, char *buffer) {
    for (int i = 0; i < current_indent * INDENT_WIDTH; i++) {
        fprintf(file, " ");
    }
    fprintf(file, "%s\n", buffer);
}

void write_header(FILE *file) {
    char header[] =
        "section .text\n"
        "global _start\n\n"
        "_start:";
    write(file, header);
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


int codegen(AST_Node *ast, Symbol_Table *table, FILE *out_file) {
    write_header(out_file);
    assign_addrs(table);
    return 0;
}
