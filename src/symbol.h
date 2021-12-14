#ifndef SYMBOL_H
#define SYMBOL_H

#include "lexer.h"

//universal list used in symbol table

typedef struct List_Container {
    struct List_Container *next;
    void *item;
} List_Container;

List_Container *new_list_container(void *item);

void free_list_container(List_Container *container);

typedef struct {
    List_Container *root, *current;
} List;

List *new_list();

//'light' free, only free list itself, not contents
void free_list(List *list);

void list_add(List *list, void *item);

//symbol table

typedef enum {
    SYM_INT, SYM_FUNC,
} Symbol_Type;

typedef struct Symbol {
    Symbol_Type type;
    Token *name;
    struct Symbol *next;
} Symbol;

//TODO no need to have 'public' headers

Symbol *new_symbol(Symbol_Type type, Token *name);

void free_symbol(Symbol *symbol);

typedef struct Scope {
    Symbol *symbols;
    struct Scope *children, *next;
} Scope;

//TODO no need to have 'public' headers

Scope *new_scope();

void free_scope(Scope *scope);

void scope_add_symbol(Scope *scope, Symbol *symbol);

typedef struct {
    Scope *root, *current; //TODO current should be a stack
} Symbol_Table;

Symbol_Table *new_symbol_table();

void free_symbol_table(Symbol_Table *table);

//create new scope
void push(Symbol_Table *table);

//return to parent scope
void pop(Symbol_Table *table);

//set new symbol to current scope
void set(Symbol_Type *table, Symbol *symbol);

//get symbol by name from current scope
Symbol *get(Symbol_Table *table, Token *name);

#endif
