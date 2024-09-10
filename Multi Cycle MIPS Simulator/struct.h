#ifndef STRUCT_H
#define STRUCT_H

#include <stdbool.h>

struct _instruction
{
    int instruction;
    unsigned char opcode;
    unsigned char rs;
    unsigned char rt;
    unsigned char rd;
    unsigned char shamt;
    unsigned char func;
    unsigned short int imm_;
    unsigned int address;
};

struct _register
{
    int data;
    bool valid;
    unsigned char tag;
};

struct _ifid
{
    int pc;
    int pc4;
    struct _instruction inst;

    bool valid;
    bool fetch_src;
};

struct _idex
{
    int address;
    int pc;
    int pc4;
    int v1;
    int v2;
    int s_imm;
    int write_reg;
    int ext_imm;
    
    // ? Control Signals
    bool isJr;
    bool isJump;
    bool isBeq;
    bool isBne;
    bool isJal;
    bool isSet;
    bool isLui;
    int reg_write;
    int mem_write;
    int mem_read;
    int mem_to_reg;
    int ALU_src;
    char ALU_op;

    unsigned char rs;
    unsigned char rt;
    // int instIndex;

    bool valid;
    bool fetch_src;
};

struct _exmem
{
    int pc4;
    int v2;
    int write_reg;
    int ext_imm;
    int ALU_out;
    unsigned char set_cond;

    bool isJal;
    bool isSet;
    bool isLui;
    int reg_write;
    int mem_write;
    int mem_read;
    int mem_to_reg;

    bool valid;
};

struct _memwb
{
    int pc4;
    int write_reg;
    int ext_imm;
    int ALU_out;
    int mem_out;
    unsigned char set_cond;

    bool isJal;
    bool isSet;
    bool isLui;
    int reg_write;
    int mem_to_reg;

    bool valid;
};

struct _forwardingUnit
{
    unsigned char rs;
    unsigned char rt;
    int from_MEM;
    int from_WB;
    int ForwardA;
    int ForwardB;
    int ForwardC;
    int mem_write;
    bool reg_write_MEM;
    bool reg_write_WB;

    int ALU_src;
    int src_input;
    unsigned char i1;
};

struct _BTB
{
    int pc;
    int target;
    bool HB;
};

#endif