#include "history.h"
#include <string.h>

void history_init(History *h) { h->count = 0; }

void history_clear(History *h) { h->count = 0; }

size_t history_count(const History *h) { return h->count; }

const HistoryEntry *history_get(const History *h, size_t index) {
    if (index >= h->count) return NULL;
    return &h->entries[index];
}

void history_add(History *h, const char *expr, double result) {
    if (h->count == HISTORY_MAX) {
        for (size_t i = 1; i < HISTORY_MAX; i++)
            h->entries[i - 1] = h->entries[i];
        h->count--;
    }
    HistoryEntry *e = &h->entries[h->count];
    strncpy(e->expr, expr, HISTORY_EXPR_LEN - 1);
    e->expr[HISTORY_EXPR_LEN - 1] = '\0';
    e->result = result;
    h->count++;
}
