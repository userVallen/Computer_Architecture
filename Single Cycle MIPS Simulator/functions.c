#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "variables.h"
#include "instruction.h"

void show_stats()
{
    printf("Displaying all registers:\n");
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j <= 24 ; j+=8)
        {
            printf("reg[%-2d]: %-8d 0x%-16x", (i + j), reg[i + j], reg[i + j]);
        }
        printf("\n");
    }
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nNumber of instructions executed: %d\nNumber of R-type instructions executed: %d\nNumber of I-type instructions executed: %d\nNumber of J-type instructions executed: %d\nNumber of memory access instructions executed: %d\nNumber of branches taken: %d\n", inst_count, r_count, i_count, j_count, mem_count, br_count);
}

int select_MUX(int control, int argCount, ...)
{
    va_list args;
    va_start(args, argCount);

    for(int i = 0; i < argCount; i++)
    {
        if(control == i)
        {
            return va_arg(args, int);
        }
        else va_arg(args, int);
    }
    va_end(args);

    return -1;
}

int set_ALU_op(char control, int arg1, int arg2)
{
    switch(control)
    {
        case '+':
            return (arg1 + arg2);
            break;

        case '-':
            return (arg1 - arg2);
            break;

        case '*':
            return (arg1 * arg2);
            break;

        case '/':
            // ! Exception for division by zero
            if(!arg2)
            {
                fprintf(stderr, "EXCEPTION OCCURED: Division by zero\n");
                exit(EXIT_FAILURE);
            }
            return (arg1 / arg2);
            break;

        case '&':
            return (arg1 & arg2);
            break;

        case '|':
            return (arg1 | arg2);
            break;

        case '~':
            return ~(arg1 | arg2);
            break;

        case '<':
            return (arg1 << arg2);
            break;

        case '>':
            return (arg1 >> arg2);
            break;

        default:
            perror("EXCEPTION OCCURED: Unknown operator\n");
            exit(EXIT_FAILURE);
            break;
    }
}

void assign_control_signals(struct _instruction *inst)
{
        // * isJr
    inst->isJr = (inst->opcode == 0x0 && (inst->func == 0x08 || inst->func == 0x09)) ? true : false;
        // * isJump
    inst->isJump = (inst->opcode == 0x2 || inst->opcode == 0x3) ? true : false;
        // * isBranch
    inst->isBranch = (inst->opcode == 0x4 || inst->opcode == 0x5) ? true : false;
        // * isJal
    inst->isJal = (inst->opcode == 0x3 || ((inst->opcode == 0x0) && (inst->func == 0x09))) ? true : false;
        // * isSet
    inst->isSet = (inst->opcode == 0xa || inst->opcode == 0xb || ((inst->opcode == 0x0) && ((inst->func == 0x2a) || (inst->func == 0x2b)))) ? true : false;
        // * isLui
    inst->isLui = (inst->opcode == 0xf) ? true : false;
        // * isShift
    inst->isShift = (inst->opcode == 0x0 && ((inst->func == 0x00) || (inst->func == 0x02))) ? true : false;
        // * reg_dst
    if(inst->opcode == 0x0 && !inst->isJr) inst->reg_dst = 1;
    else if(inst->opcode == 0x3 || ((inst->opcode == 0x0) && (inst->func == 0x09))) inst->reg_dst = 2;
    else inst->reg_dst = 0;
        // * reg_write
    inst->reg_write = ((inst->opcode == 0x0 && inst->func == 0x08) || inst->opcode == 0x2 || inst->opcode == 0x4 || inst->opcode == 0x5 || inst->opcode == 0x2b) ? 0 : 1;
        // * mem_read
    inst->mem_read = (inst->opcode == 0x23) ? 1 : 0;
        // * mem_write
    inst->mem_write = (inst->opcode == 0x2b) ? 1 : 0;
        // * mem_to_reg
    inst->mem_to_reg = (inst->opcode == 0x23) ? 1 : 0;
        // * ALU_src
    if(inst->opcode == 0x8 || inst->opcode == 0x9 || inst->opcode == 0xa || inst->opcode == 0xb || inst->opcode == 0x23 || inst->opcode == 0x2b) inst->ALU_src = 1;
    else if(inst->opcode == 0xc || inst->opcode == 0xd) inst->ALU_src = 2;
    else inst->ALU_src = 0;
        // * ALU_op
    if(inst->opcode == 0x8 || inst->opcode == 0x9 || inst->opcode == 0x23 || inst->opcode == 0x2b || ((inst->opcode == 0x0) && ((inst->func == 0x20) || (inst->func == 0x21)))) inst->ALU_op = '+';
    if(inst->opcode == 0x0 && ((inst->func == 0x22) || (inst->func == 0x23))) inst->ALU_op = '-';
    if(inst->opcode == 0xc || ((inst->opcode == 0x0) && (inst->func == 0x24))) inst->ALU_op = '&';
    if(inst->opcode == 0xd || ((inst->opcode == 0x0) && (inst->func == 0x25))) inst->ALU_op = '|';
    if(inst->opcode == 0x0 && inst->func == 0x00) inst->ALU_op = '<';
    if(inst->opcode == 0x0 && inst->func == 0x02) inst->ALU_op = '>';
    if(inst->opcode == 0x0 && inst->func == 0x27) inst->ALU_op = '~';
}

// * evaluate (both at a time?) set and branch condition
void check_conditions(bool *br_cond, bool *set_cond, int reg[], struct _instruction *inst)
{
    // * in the case of branch instructions
    if(inst->isBranch)
    {
        // * if the instruction is BEQ
        if(inst->opcode == 0x4)
        {
            *br_cond = (reg[inst->rs] == reg[inst->rt]) ? true : false;
        }

        // * if the instruction is BNE
        if(inst->opcode == 0x5)
        {
            *br_cond = (reg[inst->rs] != reg[inst->rt]) ? true : false;
        }
    }
    else *br_cond = false;

    // * in the case of set instructions
    if(inst->isSet)
    {
        *set_cond = (reg[inst->rs] < inst->s_imm) ? true : false;
    }
    else *set_cond = false;
}

void classify_instruction(unsigned int *j_count, unsigned int *i_count, unsigned int *r_count, unsigned int *mem_count, struct _instruction *inst)
{
    if(inst->opcode == 0x2 || inst->opcode == 0x3 || ((inst->opcode == 0x0) && (inst->func == 0x08)) || ((inst->opcode == 0x0) && (inst->func == 0x09))) *j_count += 1;
    if(inst->opcode == 0x4 || inst->opcode == 0x5 || inst->opcode == 0x8 || inst->opcode == 0x9 || inst->opcode == 0xa || inst->opcode == 0xb || inst->opcode == 0xc || inst->opcode == 0xd || inst->opcode == 0xf || inst->opcode == 0x23 || inst->opcode == 0x2b) *i_count += 1;
    if(inst->opcode == 0x0) *r_count += 1;
    if(inst->opcode == 0x23 || inst->opcode == 0x2b) *mem_count += 1;
}
