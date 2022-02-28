#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "parser.h"
#include "symbol.h"

int codegen(AST_Node *ast_root, Symbol_Table *table, FILE *out_file);

#endif
