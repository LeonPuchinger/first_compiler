#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("ERROR: missing input!\n");
        return 1;
    }

    struct stat st = { 0 };
    if (stat("/out", &st) == -1) {
        if (mkdir("out", 0777) == -1) {
            printf("ERROR: could not create output directory!\n");
            return 1;
        }
    }

    int input = atoi(argv[1]);

    FILE *asm_file = fopen("out/out.asm", "w");
    fprintf(asm_file, "section .text\n");
    fprintf(asm_file, "global _start\n\n");
    fprintf(asm_file, "_start:\n");
    fprintf(asm_file, "mov rax, 60\n");
    fprintf(asm_file, "mov rdi, %d\n", input);
    fprintf(asm_file, "syscall\n");
    fclose(asm_file);

    system("nasm -o out/out.o -f elf64 out/out.asm");
    system("ld -o out/out out/out.o");

    return 0;
}
