#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "lexer.h"
#include "parser.h"
#include "symbol.h"
#include "analysis.h"
#include "codegen.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("ERROR: please specify input file!\n");
        return 1;
    }

    struct stat st = { 0 };
    if (stat("out", &st) == -1) {
        if (mkdir("out", 0777) == -1) {
            printf("ERROR: could not create output directory!\n");
            return 1;
        }
    }

    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    char *text = malloc(file_size);
    fread(text, file_size, sizeof(char), file);
    fclose(file);

    //TODO use unified/consistent error handling for each compiler step

    int err;

    Token_List *tokens = new_token_list();
    err = tokenize(text, file_size, tokens);
    if (err) {
        printf("Error while running lexer\n");
        return 1;
    }

    AST_Node *ast = parse(tokens);

    Symbol_Table *table = new_symbol_table();
    err = semantic_analysis(ast, table);
    if (err) {
        printf("Error while running semantic analysis\n");
        return 1;
    }

    FILE *asm_file = fopen("out/out.asm", "w");
    err = codegen(ast, table, asm_file);
    if (err) {
        printf("Error while running codegen\n");
        return 1;
    }
    fclose(asm_file);

    system("nasm -o out/out.o -f elf64 out/out.asm");
    system("ld -o out/out out/out.o");

    return 0;
}
