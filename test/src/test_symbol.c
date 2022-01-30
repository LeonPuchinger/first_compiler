#include <stdlib.h>
#include "test.h"
#include "../../src/symbol.h"
#include "../../src/lexer.h"

//Fixtures

Symbol_Table *empty_table() {
    return new_symbol_table();
}

//Tests

int test_set() {
    int err;
    Symbol_Table *table = empty_table();

    Token *a_tk = new_token(TK_IDENT, "a", 1);
    Symbol *a_sym = new_symbol(SYM_INT, a_tk);
    symbol_table_set(table, a_sym);
    Symbol *a_return = symbol_table_get(table, a_tk);
    err = assert(a_sym, a_return);
    if (err) return err;

    return 0;
}


int main() {
    gather_tests(
        test_set,
        NULL
    );
}
