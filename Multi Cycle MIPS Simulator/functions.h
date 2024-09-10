#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>
#include "struct.h"

void showStats();
int select_MUX(unsigned char control, int arg_count, ...);
int set_ALU_op(char control, int arg1, int arg2);
void pass_latches(struct _ifid IFID[], struct _idex IDEX[], struct _exmem EXMEM[], struct _memwb MEMWB[]);
void initialize_registers(struct _register reg[]);
void initialize_branch_predictor(unsigned int *BHR, unsigned char PHT[], bool branch_outcomes[]);
void decode_instruction(struct _idex IDEX[], struct _ifid IFID[]);
void classify_instruction(unsigned int *j_count, unsigned int *i_count, unsigned int *r_count, unsigned int *mem_Count, struct _ifid IFID[]);
void identify_instruction(struct _idex IDEX[], struct _register reg[], struct _ifid IFID[], int target);
void assign_control_signals(struct _idex IDEX[], struct _ifid IFID[]);
void set_target(struct _idex IDEX[], bool *br_cond, unsigned int *br_count, int br_addr, int j_addr, int *target, int ALU_in1, int ALU_in2);
void check_forwarding(struct _forwardingUnit *forwarding_unit, struct _idex IDEX[], struct _exmem EXMEM[], struct _register reg[], int *ALU_in1, int *ALU_in2, int *write_data);
void check_stall(struct _ifid IFID[], struct _idex IDEX[], struct _exmem EXMEM[], struct _memwb MEMWB[], struct _register reg[], bool isShift, int i1, int i2);
void do_IF(struct _ifid IFID[], int mem[], int pc);
void do_ID(struct _idex IDEX[], struct _ifid IFID[], int *target, unsigned char *i1, unsigned char *i2, int *d1, int *d2);
void do_EX(struct _exmem EXMEM[], struct _idex IDEX[], struct _memwb MEMWB[], struct _ifid IFID[], int *src_input, int *j_addr, int *br_addr);
void do_MEM(struct _memwb MEMWB[], struct _exmem EXMEM[], int mem[], struct _forwardingUnit *forwarding_unit);
void do_WB(int *pc8, int *write_data, struct _memwb MEMWB[], int mem[], struct _register reg[], struct _forwardingUnit *forwarding_unit);
unsigned char update_counter(unsigned char counter, bool br_cond);
void predict_branch(unsigned int *BHR, unsigned char PHT[], bool br_cond);
void write_BTB(struct _BTB *BTB_entry, int pc, int target, bool HB);
bool check_BTB(int pc, struct _BTB BTB[], unsigned int BTB_last_entry, unsigned int *index, bool *notFound);
void flushIF(struct _ifid IFID[]);
void flushID(struct _idex IDEX[], unsigned char *i1, unsigned char *i2, int *d1, int *d2);

#endif