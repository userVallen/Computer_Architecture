#ifndef VARIABLES_H
#define VARIABLES_H

extern int mem[];
extern int pc;
extern int reg[];
extern unsigned int inst_count, r_count, i_count, j_count, mem_count, br_count;
extern int RegDst, MemRead, MemtoReg, MemWrite, ALUSrc, RegWrite;
extern char ALUOp;
extern bool isJump, isBranch, isJr, isJal, isShift, isSet, isUnsigned, isLui;

#endif