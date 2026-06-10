#include "history.h"
#include "test_util.h"
#include <string.h>
#include <stdio.h>

int main(void) {
    History h;
    history_init(&h);
    CHECK(history_count(&h) == 0);
    CHECK(history_get(&h, 0) == NULL);

    history_add(&h, "2+2", 4);
    history_add(&h, "3*3", 9);
    CHECK(history_count(&h) == 2);
    CHECK(strcmp(history_get(&h, 0)->expr, "2+2") == 0);
    CHECK_DBL(history_get(&h, 0)->result, 4);
    CHECK(strcmp(history_get(&h, 1)->expr, "3*3") == 0);
    CHECK_DBL(history_get(&h, 1)->result, 9);

    history_clear(&h);
    CHECK(history_count(&h) == 0);

    // límite: al superar HISTORY_MAX se descarta la más antigua
    for (int i = 0; i < HISTORY_MAX + 5; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", i);
        history_add(&h, buf, i);
    }
    CHECK(history_count(&h) == HISTORY_MAX);
    // la más antigua ahora debe ser "5" (se descartaron 0..4)
    CHECK(strcmp(history_get(&h, 0)->expr, "5") == 0);

    TEST_REPORT();
}
