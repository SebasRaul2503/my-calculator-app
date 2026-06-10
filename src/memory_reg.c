#include "memory_reg.h"

void memreg_init(MemoryReg *m)  { m->value = 0.0; m->has_value = false; }
void memreg_clear(MemoryReg *m) { m->value = 0.0; m->has_value = false; }
double memreg_recall(const MemoryReg *m) { return m->value; }
void memreg_add(MemoryReg *m, double x) { m->value += x; m->has_value = true; }
void memreg_sub(MemoryReg *m, double x) { m->value -= x; m->has_value = true; }
bool memreg_has_value(const MemoryReg *m) { return m->has_value; }
