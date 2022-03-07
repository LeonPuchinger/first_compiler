#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "../../src/symbol.h"
#include "../../src/lexer.h"

//Fixtures

Symbol_Table *empty_table() {
    return new_symbol_table();
}

Symbol *symbol_ident(char *token_content) {
    Token *a_tk = new_token(TK_IDENT, token_content, strlen(token_content));
    return new_symbol(SYM_INT, a_tk);
}

Symbol_Table *populated_table() {
    Symbol_Table *table = empty_table();
    symbol_table_set(table, symbol_ident("a"));

    symbol_table_push(table);
    symbol_table_set(table, symbol_ident("b"));
    symbol_table_pop(table);

    symbol_table_push(table);
    symbol_table_set(table, symbol_ident("c"));

    return table;
}

//Tests

int test_set_get() {
    int err;
    Symbol_Table *table = empty_table();

    Symbol *a_sym = symbol_ident("a");
    symbol_table_set(table, a_sym);
    Symbol *a_return = symbol_table_get(table, a_sym->name);
    err = assert(a_sym, a_return);
    if (err) return err;

    return 0;
}

int test_pushed_scopes() {
    int err;
    Symbol_Table *table = empty_table();

    Symbol *a_sym = symbol_ident("a");
    Symbol *b_sym = symbol_ident("b");
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

    Symbol *a_sym = symbol_ident("a");
    Symbol *b_sym = symbol_ident("b");
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

int test_reset_table() {
    int err;
    Symbol_Table *table = populated_table();

    symbol_table_reset_current(table);

    Symbol *a_return = symbol_table_get(table, symbol_ident("a")->name);
    err = assert_not(a_return, NULL);
    if (err) return err;

    Symbol *b_return = symbol_table_get(table, symbol_ident("b")->name);
    err = assert(b_return, NULL);
    if (err) return err;

    return 0;
}

int test_walk_child() {
    int err;
    Symbol_Table *table = populated_table();
    symbol_table_reset_current(table);

    symbol_table_walk_child(table);

    Symbol *b_return = symbol_table_get(table, symbol_ident("b")->name);
    err = assert_not(b_return, NULL);
    if (err) return err;

    return 0;
}

//dependent on success of test_reset_table & test_walk_child
int test_walk_next() {
    int err;
    Symbol_Table *table = populated_table();
    symbol_table_reset_current(table);
    symbol_table_walk_child(table);

    symbol_table_walk_next(table);

    Symbol *b_return = symbol_table_get(table, symbol_ident("b")->name);
    err = assert(b_return, NULL);
    if (err) return err;

    Symbol *c_return = symbol_table_get(table, symbol_ident("c")->name);
    err = assert_not(c_return, NULL);
    if (err) return err;

    return 0;
}

int test_symbol_table_is_local() {
    int err;
    Symbol_Table *table = populated_table();

    //child scope
    int is_local = 0;
    is_local = symbol_table_is_local(table, symbol_ident("c")->name) != NULL;
    err = assert_int_not(is_local, 0);
    if (err) return err;

    //root scope
    symbol_table_reset_current(table);
    is_local = symbol_table_is_local(table, symbol_ident("a")->name) != NULL;
    err = assert_int_not(is_local, 0);
    if (err) return err;

    return 0;
}

int main() {
    gather_tests(
        test_set_get,
        test_pushed_scopes,
        test_sibling_scopes,
        test_reset_table,
        test_walk_child,
        test_walk_next,
        test_symbol_table_is_local,
        NULL
    );
}
