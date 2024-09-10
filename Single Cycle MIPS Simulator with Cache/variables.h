#ifndef VARIABLES_H
#define VARIABLES_H

#define WAY_COUNT 4
#define SET_COUNT 16
#include <stdbool.h>
#include "struct.h"

extern int mem[];
extern int pc;
extern int reg[];
extern struct _instruction inst;
extern struct _cache cache[WAY_COUNT][SET_COUNT];
extern struct _cache *oldest_cache_line[SET_COUNT];
extern struct _cache *victim_cache_line; 
extern unsigned int inst_count, r_count, i_count, j_count, mem_count, br_count;
extern int reg_dst, mem_read, mem_to_reg, mem_write, ALU_src, reg_write;
extern char ALU_op;
extern bool isJump, isBranch, isJr, isJal, isShift, isSet, isLui, isUnsigned;
extern int write_reg, write_data, ALU_in, mem_out, target, ALU_out;
extern bool br_cond, set_cond;
extern int out_data;
extern bool cache_hit;
extern unsigned int tag;
extern unsigned char idx, offset, line_index, plane_index, oldest_index;
extern unsigned int hit_count, miss_count;

#endif