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
    unsigned short imm;
    int s_imm;
    unsigned int ext_imm;
};

struct _cache
{
    bool valid;
    bool dirty;
    bool sca;
    unsigned int tag;
    int data[4];
    unsigned char order_of_entrance;
    int starting_address;
};

#endif