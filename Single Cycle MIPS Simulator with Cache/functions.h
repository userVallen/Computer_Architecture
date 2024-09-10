#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "struct.h"

void showStats();
int select_MUX(int control, int argCount, ...);
int set_ALU_op(char control, int arg1, int arg2);
void reset_state();
void classify_instruction();
void decode_instruction();
void execute_instruction();
void set_output();
void set_target();
void assign_control_signals();
void initialize_registers();
void initialize_cache();
int read_mem(int address);
void write_mem(int address, int value);
void check_cache();
int read_cache();
void write_cache(struct _cache *cache_line);
unsigned char find_oldest();
void find_victim();
void update_order();
void divide_address();
// void check_dirty(struct _cache *cache_line);
// void display_set(int num);

#endif