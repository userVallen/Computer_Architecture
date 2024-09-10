#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdbool.h>
#include "struct.h"

extern int mem[];
extern int pc;
extern struct _register reg[];
extern struct _instruction inst;
extern struct _forwardingUnit forwarding_unit;
extern unsigned int inst_count, r_count, i_count, j_count, mem_count, br_count;
extern int pc8, cycle_count;
extern unsigned char reg_dst, ForwardA, ForwardB;
extern bool isShift, isUnsigned;
extern int write_data, src_input, ALU_in1, ALU_in2, target, j_addr, br_addr, pc8, d1, d2;
extern bool br_cond;
extern unsigned char set_cond, i1, i2;

#endif