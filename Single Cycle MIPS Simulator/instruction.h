#ifndef INSTRUCTION_H
#define INSTRUCTION_H

struct _instruction
{
    // * instruction values
    int instruction;
    unsigned char opcode;
    unsigned char rs;
    unsigned char rt;
    unsigned char rd;
    unsigned char shamt;
    unsigned char func;
    unsigned short int imm;
    int s_imm;
    int ext_imm;

    // * control signals
    int reg_dst;
    int mem_read;
    int mem_to_reg;
    char ALU_op;
    int mem_write;
    int ALU_src;
    int reg_write;
    bool isJump;
    bool isBranch;
    bool isJr;
    bool isJal;
    bool isShift;
    bool isSet;
    bool isUnsigned;
    bool isLui;
};

#endif