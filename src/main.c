#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "lexer.h"
#include "parser.h"

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

    /* FILE *asm_file = fopen("out/out.asm", "w");
    fprintf(asm_file, "assembly output");
    fclose(asm_file);

    system("nasm -o out/out.o -f elf64 out/out.asm");
    system("ld -o out/out out/out.o"); */

    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    char *text = malloc(file_size);
    fread(text, file_size, sizeof(char), file);
    fclose(file);

    Token_List *tokens = new_token_list();
    int err = tokenize(text, file_size, tokens);
    AST_Node *ast = parse(tokens);

    //TODO free AST
    free_token_list(tokens);
    free(text);
    return err;
}
