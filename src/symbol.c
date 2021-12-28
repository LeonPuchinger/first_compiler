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
