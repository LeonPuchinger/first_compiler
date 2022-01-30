#include <stdlib.h>
#include "symbol.h"

//collections

Collection_Container *new_collection_container(void *item) {
    Collection_Container *new = malloc(sizeof(Collection_Container));
    new->item = item;
    new->next = NULL;
    return new;
}

void free_collection_container(Collection_Container *container) {
    free(container);
}

//list

List *new_list() {
    List *new = malloc(sizeof(List));
    new->root = NULL;
    new->current = NULL;
    return new;
}

void free_list(List *list) {
    Collection_Container *current = list->root;
    while (current != NULL) {
        free_collection_container(current);
        current = current->next;
    }
    free(list);
}

void deep_free_list(List *list, void (*free_item)()) {
    Collection_Container *current = list->root;
    while (current != NULL) {
        (*free_item)(current->item);
        free_collection_container(current);
        current = current->next;
    }
    free(list);
}

void list_add(List *list, void *item) {
    Collection_Container *container = new_collection_container(item);
    if (list->root == NULL) {
        list->root = container;
    } else {
        list->current->next = container;
    }
    list->current = container;
}

//stack

Stack *new_stack() {
    Stack *new = malloc(sizeof(Stack));
    new->top = NULL;
    return new;
}

void free_stack(Stack *stack) {
    Collection_Container *current = stack->top;
    while (current != NULL) {
        free_collection_container(current);
        current = current->next;
    }
    free(stack);
}

void stack_push(Stack *stack, void *item) {
    Collection_Container *container = new_collection_container(item);
    container->next = stack->top;
    stack->top = container;
}

void *stack_pop(Stack *stack) {
    Collection_Container *container = stack->top;
    if (container == NULL) {
        return NULL;
    }
    stack->top = container->next;
    void *item = container->item;
    free_collection_container(container);
    return item;
}

void *stack_get(Stack *stack) {
    if (stack == NULL || stack->top == NULL) {
        return NULL;
    }
    return stack->top->item;
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
    Scope *root_scope = new_scope();
    new->root_scope = root_scope;
    new->current = new_stack();
    stack_push(new->current, root_scope);
    return new;
}

void free_symbol_table(Symbol_Table *table) {
    free_stack(table->current);
    free_scope(table->root_scope);
    free(table);
}

void symbol_table_push(Symbol_Table *table) {
    Scope *scope = new_scope();
    Scope *current_scope = stack_get(table->current);
    list_add(current_scope->scopes, scope);
    stack_push(table->current, scope);
}

void symbol_table_pop(Symbol_Table *table) {
    if (stack_pop(table->current) == NULL) {
        stack_push(table->current, table->root_scope);
    }
}

void symbol_table_set(Symbol_Table *table, Symbol *symbol) {
    Scope *current_scope = stack_get(table->current);
    list_add(current_scope->symbols, symbol);
}

Symbol *symbol_table_get(Symbol_Table *table, Token *name) {
    Collection_Container *current_scope_cont = table->current->top;
    while (current_scope_cont != NULL) {
        Scope *current_scope = current_scope_cont->item;
        Collection_Container *current_sym_cont = current_scope->symbols->root;
        while (current_sym_cont != NULL) {
            Symbol *current_symbol = current_sym_cont->item;
            if (current_symbol->name == name) {
                return current_symbol;
            }
            current_sym_cont = current_sym_cont->next;
        }
        current_scope_cont = current_scope_cont->next;
    }
    return NULL;
}
