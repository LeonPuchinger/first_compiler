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

int codegen(AST_Node *ast, Symbol_Table *table, FILE *out_file) {
    write_header(out_file);
    return 0;
}
