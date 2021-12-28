#include <stdlib.h>
#include "symbol.h"

//list

List_Container *new_list_container(void *item) {
    List_Container *new = malloc(sizeof(List_Container));
    new->item = item;
    new->next = NULL;
    return new;
}

void free_list_container(List_Container *container) {
    free(container);
}

List *new_list() {
    List *new = malloc(sizeof(List));
    new->root = NULL;
    new->current = NULL;
    return new;
}

void free_list(List *list) {
    List_Container *current = list->root;
    while (current != NULL) {
        free_list_container(current);
    }
    free(list);
}

void deep_free_list(List *list, void (*free_item)()) {
    List_Container *current = list->root;
    while (current != NULL) {
        (*free_item)(current->item);
        free_list_container(current);
    }
    free(list);
}

void list_add(List *list, void *item) {
    List_Container *container = new_list_container(item);
    if (list->root == NULL) {
        list->root = container;
    } else {
        list->current->next = container;
    }
    list->current = container;
}

//symbol table

Symbol *new_symbol(Symbol_Type type, Token *name) {
    Symbol *new = malloc(sizeof(Symbol));
    new->type = type;
    new->name = name;
    return new;
}

void free_symbol(Symbol *symbol) {
    free(symbol);
}

Scope *new_scope() {
    Scope *new = malloc(sizeof(Scope));
    new->symbols = new_list();
    new->scopes = new_list();
    return new;
}

void free_scope(Scope *scope) {
    deep_free_list(scope->symbols, &free_symbol);
    deep_free_list(scope->scopes, &free_scope);
    free(scope);
}

void scope_add_symbol(Scope *scope, Symbol *symbol) {
    list_add(scope->symbols, symbol);
}

void scope_add_scope(Scope *scope, Scope *add_scope) {
    list_add(scope->scopes, add_scope);
}

Symbol_Table *new_symbol_table() {
    Symbol_Table *new = malloc(sizeof(Symbol_Table));
    new->current = NULL; //TODO init stack
    new->scopes = new_list(); //TODO create root scope
}

void free_symbol_table(Symbol_Table *table) {
    //TODO free stack
    deep_free_list(table->scopes, &free_scope);
    free(table);
}
