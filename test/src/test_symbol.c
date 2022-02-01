#include <stdlib.h>
#include "test.h"
#include "../../src/symbol.h"
#include "../../src/lexer.h"

//Fixtures

Symbol_Table *empty_table() {
    return new_symbol_table();
}

Symbol *symbol_ident(char *token_content, int token_size) {
    Token *a_tk = new_token(TK_IDENT, token_content, token_size);
    return new_symbol(SYM_INT, a_tk);
}

//Tests

int test_set_get() {
    int err;
    Symbol_Table *table = empty_table();

    Symbol *a_sym = symbol_ident("a", 1);
    symbol_table_set(table, a_sym);
    Symbol *a_return = symbol_table_get(table, a_sym->name);
    err = assert(a_sym, a_return);
    if (err) return err;

    return 0;
}

int test_pushed_scopes() {
    int err;
    Symbol_Table *table = empty_table();

    Symbol *a_sym = symbol_ident("a", 1);
    Symbol *b_sym = symbol_ident("b", 1);
    Symbol *a_return, *b_return;

    symbol_table_set(table, a_sym);
    symbol_table_push(table);
    symbol_table_set(table, b_sym);

    //a should be available in pushed scope
    a_return = symbol_table_get(table, a_sym->name);
    err = assert(a_return, a_sym);
    if (err) return err;

    //b should be available in pushed scope
    b_return = symbol_table_get(table, b_sym->name);
    err = assert(b_return, b_sym);
    if (err) return err;

    symbol_table_pop(table);

    //a should be available in root scope
    a_return = symbol_table_get(table, a_sym->name);
    err = assert(a_return, a_sym);
    if (err) return err;

    //b should not be available in root scope
    b_return = symbol_table_get(table, b_sym->name);
    err = assert(b_return, NULL);
    if (err) return err;

    return 0;
}

int test_sibling_scopes() {
    int err;
    Symbol_Table *table = empty_table();

    Symbol *a_sym = symbol_ident("a", 1);
    Symbol *b_sym = symbol_ident("b", 1);
    Symbol *a_return, *b_return;

    symbol_table_push(table);
    symbol_table_set(table, a_sym);
    symbol_table_pop(table);

    symbol_table_push(table);
    symbol_table_set(table, b_sym);

    //a should not be available in sibling scope
    a_return = symbol_table_get(table, a_sym->name);
    err = assert(a_return, NULL);
    if (err) return err;

    //b should be available in sibling scope
    b_return = symbol_table_get(table, b_sym->name);
    err = assert(b_return, b_sym);
    if (err) return err;

    return 0;
}

int main() {
    gather_tests(
        test_set_get,
        test_pushed_scopes,
        test_sibling_scopes,
        NULL
    );
}
