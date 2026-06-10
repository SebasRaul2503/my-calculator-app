#ifndef MEMORY_REG_H
#define MEMORY_REG_H

#include <stdbool.h>

typedef struct {
    double value;
    bool has_value;
} MemoryReg;

void memreg_init(MemoryReg *m);
void memreg_clear(MemoryReg *m);            // MC
double memreg_recall(const MemoryReg *m);   // MR
void memreg_add(MemoryReg *m, double x);    // M+
void memreg_sub(MemoryReg *m, double x);    // M-
bool memreg_has_value(const MemoryReg *m);

#endif
