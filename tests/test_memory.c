#include "memory_reg.h"
#include "test_util.h"

int main(void) {
    MemoryReg m;
    memreg_init(&m);
    CHECK(!memreg_has_value(&m));
    CHECK_DBL(memreg_recall(&m), 0);

    memreg_add(&m, 5);
    CHECK(memreg_has_value(&m));
    CHECK_DBL(memreg_recall(&m), 5);

    memreg_sub(&m, 2);
    CHECK_DBL(memreg_recall(&m), 3);

    memreg_clear(&m);
    CHECK(!memreg_has_value(&m));
    CHECK_DBL(memreg_recall(&m), 0);

    // tras operar a cero, sigue "en uso"
    memreg_add(&m, 4);
    memreg_sub(&m, 4);
    CHECK(memreg_has_value(&m));
    CHECK_DBL(memreg_recall(&m), 0);

    TEST_REPORT();
}
