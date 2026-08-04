#include "bc2_receive_request.h"

#include <assert.h>

int main(void) {
    assert(bc2_receive_address_is_valid("bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"));
    assert(!bc2_receive_address_is_valid("tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kg3g4ty"));
    assert(!bc2_receive_address_is_valid("bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t5"));
    assert(!bc2_receive_address_is_valid("BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4"));
    return 0;
}
