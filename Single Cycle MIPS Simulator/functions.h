#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>
#include "instruction.h"

void show_stats();
int select_MUX(int control, int argCount, ...);
int set_ALU_op(char control, int arg1, int arg2);
// void resetState();
void assign_control_signals(struct _instruction *inst);
void check_conditions(bool *br_cond, bool *set_cond, int reg[], struct _instruction *inst);
void classify_instruction(unsigned int *j_count, unsigned int *i_count, unsigned int *r_count, unsigned int *mem_count, struct _instruction *inst);

#endif