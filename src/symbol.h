#ifndef SYMBOL_H
#define SYMBOL_H

#include "lexer.h"

//universal collections used in symbol table

typedef struct Collection_Container {
    struct Collection_Container *next;
    void *item;
} Collection_Container;

Collection_Container *new_collection_container(void *item);

void free_collection_container(Collection_Container *container);

//universal list

typedef struct {
    Collection_Container *root, *current;
} List;

List *new_list();

//only free list itself, not contents
void free_list(List *list);

//free list and contents aswell
void deep_free_list(List *list, void (*free_item)());

void list_add(List *list, void *item);

//universal stack

typedef struct {
    Collection_Container *top;
} Stack;

Stack *new_stack();

//only free stack itself, not contents
void free_stack(Stack *stack);

void stack_push(Stack *stack, void *item);

void *stack_pop(Stack *stack);

void *stack_get(Stack *stack);

//symbol table

typedef enum {
    SYM_INT, SYM_FUNC,
} Symbol_Type;

typedef struct Symbol {
    Symbol_Type type;
    Token *name;
    int addr;
} Symbol;

//TODO no need to have 'public' headers

Symbol *new_symbol(Symbol_Type type, Token *name);

void free_symbol(Symbol *symbol);

typedef struct Scope {
    List *symbols, *scopes;
} Scope;

//TODO no need to have 'public' headers

Scope *new_scope();

void free_scope(Scope *scope);

void scope_add_symbol(Scope *scope, Symbol *symbol);

void scope_add_scope(Scope *scope, Scope *add_scope);

typedef struct {
    Stack *current;
    Scope *root_scope;
} Symbol_Table;

Symbol_Table *new_symbol_table();

void free_symbol_table(Symbol_Table *table);

//create new scope
void symbol_table_push(Symbol_Table *table);

//return to parent scope
void symbol_table_pop(Symbol_Table *table);

//set new symbol to current scope
void symbol_table_set(Symbol_Table *table, Symbol *symbol);

//get symbol by name from current scope
Symbol *symbol_table_get(Symbol_Table *table, Token *name);

#endif
