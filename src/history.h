#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>

#define HISTORY_MAX 100
#define HISTORY_EXPR_LEN 256

typedef struct {
    char expr[HISTORY_EXPR_LEN];
    double result;
} HistoryEntry;

typedef struct {
    HistoryEntry entries[HISTORY_MAX];
    size_t count;   // entradas válidas; index 0 = más antigua
} History;

void history_init(History *h);
void history_add(History *h, const char *expr, double result);
size_t history_count(const History *h);
const HistoryEntry *history_get(const History *h, size_t index);
void history_clear(History *h);

#endif
