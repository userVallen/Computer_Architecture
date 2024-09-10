#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "variables.h"
#include "struct.h"
#define MEM_SIZE 0x400000
#define REG_COUNT 32
#define OUTCOME_COUNT 10
#define PHT_COUNT 4

void showStats()
{
    printf("Displaying all registers:\n");
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j <= 24 ; j+=8)
        {
            printf("reg[%02d]: %-8d 0x%-16x", (i + j), reg[i + j].data, reg[i + j].data);
        }
        printf("\n");
    }
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nNumber of instructions executed: %d\nNumber of R-type instructions executed: %d\nNumber of I-type instructions executed: %d\nNumber of J-type instructions executed: %d\nNumber of memory access instructions executed: %d\nNumber of branches taken: %d\n\n", inst_count, r_count, i_count, j_count, mem_count, br_count);
}

int select_MUX(unsigned char control, int arg_count, ...)
{
    va_list args;
    va_start(args, arg_count);

    for(int i = 0; i < arg_count; i++)
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
                fprintf(stderr, "EXCEPTION OCCURRED: Division by zero\n");
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
            fprintf(stderr, "EXCEPTION OCCURRED: Unknown operator --> %c\n", control);
            exit(EXIT_FAILURE);
            break;
    }
}

void pass_latches(struct _ifid IFID[], struct _idex IDEX[], struct _exmem EXMEM[], struct _memwb MEMWB[])
{
    IFID[1].pc = IFID[0].pc;
    IFID[1].pc4 = IFID[0].pc4;
    IFID[1].inst = IFID[0].inst;
    IFID[1].valid = IFID[0].valid;
    IFID[1].fetch_src = IFID[0].fetch_src;

    IDEX[1].address = IDEX[0].address;
    IDEX[1].pc = IDEX[0].pc;
    IDEX[1].pc4 = IDEX[0].pc4;
    IDEX[1].v1 = IDEX[0].v1;
    IDEX[1].v2 = IDEX[0].v2;
    IDEX[1].s_imm = IDEX[0].s_imm;
    IDEX[1].write_reg = IDEX[0].write_reg;
    IDEX[1].ext_imm = IDEX[0].ext_imm;
    IDEX[1].isJr = IDEX[0].isJr;
    IDEX[1].isJump = IDEX[0].isJump;
    IDEX[1].isBeq = IDEX[0].isBeq;
    IDEX[1].isBne = IDEX[0].isBne;
    IDEX[1].isJal = IDEX[0].isJal;
    IDEX[1].isSet = IDEX[0].isSet;
    IDEX[1].isLui = IDEX[0].isLui;
    IDEX[1].reg_write = IDEX[0].reg_write;
    IDEX[1].mem_write = IDEX[0].mem_write;
    IDEX[1].mem_read = IDEX[0].mem_read;
    IDEX[1].mem_to_reg = IDEX[0].mem_to_reg;
    IDEX[1].ALU_src = IDEX[0].ALU_src;
    IDEX[1].ALU_op = IDEX[0].ALU_op;
    IDEX[1].rs = IDEX[0].rs;
    IDEX[1].rt = IDEX[0].rt;
    IDEX[1].valid = IDEX[0].valid;
    IDEX[1].fetch_src = IDEX[0].fetch_src;

    EXMEM[1].pc4 = EXMEM[0].pc4;
    EXMEM[1].v2 = EXMEM[0].v2;
    EXMEM[1].write_reg = EXMEM[0].write_reg;
    EXMEM[1].ext_imm = EXMEM[0].ext_imm;
    EXMEM[1].set_cond = EXMEM[0].set_cond;
    EXMEM[1].ALU_out = EXMEM[0].ALU_out;
    EXMEM[1].isJal = EXMEM[0].isJal;
    EXMEM[1].isSet = EXMEM[0].isSet;
    EXMEM[1].isSet = EXMEM[0].isSet;
    EXMEM[1].isLui = EXMEM[0].isLui;
    EXMEM[1].reg_write = EXMEM[0].reg_write;
    EXMEM[1].mem_write = EXMEM[0].mem_write;
    EXMEM[1].mem_read = EXMEM[0].mem_read;
    EXMEM[1].mem_to_reg = EXMEM[0].mem_to_reg;
    EXMEM[1].valid = EXMEM[0].valid;

    MEMWB[1].pc4 = MEMWB[0].pc4;
    MEMWB[1].write_reg = MEMWB[0].write_reg;
    MEMWB[1].ext_imm = MEMWB[0].ext_imm;
    MEMWB[1].set_cond = MEMWB[0].set_cond;
    MEMWB[1].ALU_out = MEMWB[0].ALU_out;
    MEMWB[1].mem_out = MEMWB[0].mem_out;
    MEMWB[1].isJal = MEMWB[0].isJal;
    MEMWB[1].isSet = MEMWB[0].isSet;
    MEMWB[1].isLui = MEMWB[0].isLui;
    MEMWB[1].reg_write = MEMWB[0].reg_write;
    MEMWB[1].mem_to_reg = MEMWB[0].mem_to_reg;
    MEMWB[1].valid = MEMWB[0].valid;
}

void initialize_registers(struct _register reg[])
{
    for(int i = 0; i < 32; i++) 
    {
        reg[i].data = 0;
        reg[i].valid = true;
    }
    reg[29].data = 0x1000000;
    reg[31].data = 0xffffffff;
}

void initialize_branch_predictor(unsigned char *BHR, unsigned char PHT[], bool branch_outcomes[])
{
    *BHR = 0; // * Initial value 00
    for(int i = 0; i < PHT_COUNT; i++) PHT[i] = 1; // * Initialize all PHT entries to "Weakly Not Taken" (01 or 1 in decimal)
    for(int i = 0; i < OUTCOME_COUNT; i++) branch_outcomes[i] = 1; // * Initialize branch outcomes (assume all are taken)
}

void decode_instruction(struct _idex IDEX[], struct _ifid IFID[])
{
    IFID[1].inst.opcode = (IFID[1].inst.instruction >> 26) & 0x3f;
    IFID[1].inst.rs = (IFID[1].inst.instruction >> 21) & 0x1f;
    IFID[1].inst.rt = (IFID[1].inst.instruction >> 16) & 0x1f;
    IFID[1].inst.rd = (IFID[1].inst.instruction >> 11) & 0x1f;
    IFID[1].inst.shamt = (IFID[1].inst.instruction >> 6) & 0x1f;
    IFID[1].inst.func = IFID[1].inst.instruction & 0x3f;
    IFID[1].inst.imm_ = IFID[1].inst.instruction & 0xffff;
    IFID[1].inst.address = (IFID[1].inst.instruction & 0x3ffffff);
    IDEX[0].s_imm = (IFID[1].inst.imm_ & 0x8000) ? (IFID[1].inst.imm_ | 0xffff0000) : (IFID[1].inst.imm_);
    IDEX[0].ext_imm = IFID[1].inst.imm_ << 16;
}

void classify_instruction(unsigned int *j_count, unsigned int *i_count, unsigned int *r_count, unsigned int *mem_count, struct _ifid IFID[])
{
    if(IFID[1].inst.opcode == 0x2 || IFID[1].inst.opcode == 0x3 || ((IFID[1].inst.opcode == 0x0) && (IFID[1].inst.func == 0x08)) || ((IFID[1].inst.opcode == 0x0) && (IFID[1].inst.func == 0x09))) *j_count += 1;
    if(IFID[1].inst.opcode == 0x4 || IFID[1].inst.opcode == 0x5 || IFID[1].inst.opcode == 0x8 || IFID[1].inst.opcode == 0x9 || IFID[1].inst.opcode == 0xa || IFID[1].inst.opcode == 0xb || IFID[1].inst.opcode == 0xc || IFID[1].inst.opcode == 0xd || IFID[1].inst.opcode == 0xf || IFID[1].inst.opcode == 0x23 || IFID[1].inst.opcode == 0x2b) *i_count += 1;
    if(IFID[1].inst.opcode == 0x0) *r_count += 1;
    if(IFID[1].inst.opcode == 0x23 || IFID[1].inst.opcode == 0x2b) *mem_count += 1;
}

void identify_instruction(struct _idex IDEX[], struct _register reg[], struct _ifid IFID[], int target)
{
    switch(IFID[1].inst.opcode)
    {
        // * J (J-type)
        case 0x2:
            printf("\n~~~\nOPCODE: J\nADDRESS: 0x%x\n~~~\n", target);
            break;

        // * JAL (J-type)
        case 0x3:
            printf("\n~~~\nOPCODE: JAL\nADDRESS: 0x%x\n~~~\n", target);
            break;

        // * BEQ (I-type)
        case 0x4:
            printf("\n~~~\nOPCODE: BEQ\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].s_imm);
            break;

        // * BNE (I-type)
        case 0x5:
            printf("\n~~~\nOPCODE: BNE\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].s_imm);
            break;

        // * ADDI (I-type)
        case 0x8:
            printf("\n~~~\nOPCODE: ADDI\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].s_imm);
            break;

        // * ADDIU (I-type)
        case 0x9:
            printf("\n~~~\nOPCODE: ADDIU\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.imm_);
            break;

        // * SLTI (I-type)
        case 0xa:
            printf("\n~~~\nOPCODE: SLTI\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].s_imm);
            break;

        // * SLTIU (I-type)
        case 0xb:
            printf("\n~~~\nOPCODE: SLTIU\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].s_imm);
            break;
        
        // * ANDI
        case 0xc:
            printf("\n~~~\nOPCODE: ANDI\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].ext_imm);
            IDEX[0].ext_imm = IFID[1].inst.imm_ | 0x00000000;
            break;

        // * ORI
        case 0xd:
            printf("\n~~~\nOPCODE: ORI\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].ext_imm);
            IDEX[0].ext_imm = IFID[1].inst.imm_ | 0x00000000;
            break;

        // * LUI
        case 0xf:
            printf("\n~~~\nOPCODE: LUI\n\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rt, IDEX[0].ext_imm);
            IDEX[0].ext_imm = IFID[1].inst.imm_ << 16;
            break;

        // * LW (I-type)
        case 0x23:
            printf("\n~~~\nOPCODE: LW\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].s_imm);
            break;

        // * SW (I-type)
        case 0x2b:
            printf("\n~~~\nOPCODE: SW\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IDEX[0].s_imm);
            break;

        // * R-type
        case 0x0:
            switch(IFID[1].inst.func)
            {
                // * SLL
                case 0x00:
                    printf("\n~~~\nOPCODE: SLL\nRT: %d\nRD: %d\nSHAMT: %d\n~~~\n", IFID[1].inst.rt, IFID[1].inst.rd, IFID[1].inst.shamt);
                    break;

                // * SRL
                case 0x02:
                    printf("\n~~~\nOPCODE: SRL\nRT: %d\nRD: %d\nSHAMT: %d\n~~~\n", IFID[1].inst.rt, IFID[1].inst.rd, IFID[1].inst.shamt);
                    break;

                // * JR
                case 0x08:
                    printf("\n~~~\nOPCODE: JR\nADDRESS: 0x%x\n~~~\n", reg[IFID[1].inst.rs].data);
                    break;

                // * JALR
                case 0x09:
                    printf("\n~~~\nOPCODE: JALR\nADDRESS: 0x%x\n~~~\n", reg[IFID[1].inst.rs].data);
                    break;

                // * ADD
                case 0x20:
                    printf("\n~~~\nOPCODE: ADD\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    break;

                // * ADDU
                case 0x21:
                    printf("\n~~~\nOPCODE: ADDU\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    isUnsigned = true;
                    break;

                // * SUB
                case 0x22:
                    printf("\n~~~\nOPCODE: SUB\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    break;

                // * SUBU
                case 0x23:
                    printf("\n~~~\nOPCODE: ADDU\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    break;

                // * AND
                case 0x24:
                    printf("\n~~~\nOPCODE: AND\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    break;

                // * OR
                case 0x25:
                    printf("\n~~~\nOPCODE: OR\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    break;

                // * NOR
                case 0x27:
                    printf("\n~~~\nOPCODE: NOR\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    break;

                // * SLT
                case 0x2a:
                    printf("\n~~~\nOPCODE: SLT\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    break;

                // * SLTU
                case 0x2b:
                    printf("\n~~~\nOPCODE: SLTU\nRS: %d\nRT: %d\nRD: %d\n~~~\n", IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd);
                    break;

                default:
                    // ! Exception for unknown command
                    fprintf(stderr, "EXCEPTION OCCURRED: Unknown command\nopcode = 0x%x\nfunc = 0x%x", IFID[1].inst.opcode, IFID[1].inst.func);
                    exit(EXIT_FAILURE);
                    break;
            }
            break;
        
        default:
            // ! Exception for unknown command
            fprintf(stderr, "EXCEPTION OCCURRED: Unknown command\nopcode = 0x%x\nfunc = 0x%x", IFID[1].inst.opcode, IFID[1].inst.func);
            exit(EXIT_FAILURE);
            break;
    }
}

void assign_control_signals(struct _idex IDEX[], struct _ifid IFID[])
{
        // * isJr
    IDEX[0].isJr = (IFID[1].inst.opcode == 0x0 && (IFID[1].inst.func == 0x08 || IFID[1].inst.func == 0x09)) ? true : false;
        // * isJump
    IDEX[0].isJump = (IFID[1].inst.opcode == 0x2 || IFID[1].inst.opcode == 0x3) ? true : false;
        // * isBeq
    IDEX[0].isBeq = (IFID[1].inst.opcode == 0x4) ? true : false;
        // * isBne
    IDEX[0].isBne = (IFID[1].inst.opcode == 0x5) ? true : false;
        // * isJal
    IDEX[0].isJal = (IFID[1].inst.opcode == 0x3 || ((IFID[1].inst.opcode == 0x0) && (IFID[1].inst.func == 0x09))) ? true : false;
        // * isSet
    IDEX[0].isSet = (IFID[1].inst.opcode == 0xa || IFID[1].inst.opcode == 0xb || ((IFID[1].inst.opcode == 0x0) && ((IFID[1].inst.func == 0x2a) || (IFID[1].inst.func == 0x2b)))) ? true : false;
        // * isLui
    IDEX[0].isLui = (IFID[1].inst.opcode == 0xf) ? true : false;
        // * isShift
    isShift = (IFID[1].inst.opcode == 0x0 && ((IFID[1].inst.func == 0x00) || (IFID[1].inst.func == 0x02))) ? true : false;
        // * reg_dst
    if(IFID[1].inst.opcode == 0x0 && !IDEX[0].isJr) reg_dst = 1;
    else if(IFID[1].inst.opcode == 0x3 || ((IFID[1].inst.opcode == 0x0) && (IFID[1].inst.func == 0x09))) reg_dst = 2;
    else reg_dst = 0;
        // * reg_write
    IDEX[0].reg_write = ((IFID[1].inst.opcode == 0x0 && IFID[1].inst.func == 0x08) || IFID[1].inst.opcode == 0x2 || IFID[1].inst.opcode == 0x4 || IFID[1].inst.opcode == 0x5 || IFID[1].inst.opcode == 0x2b) ? 0 : 1;
        // * mem_read
    IDEX[0].mem_read = (IFID[1].inst.opcode == 0x23) ? 1 : 0;
        // * mem_write
    IDEX[0].mem_write = (IFID[1].inst.opcode == 0x2b) ? 1 : 0;
        // * mem_to_reg
    IDEX[0].mem_to_reg = (IFID[1].inst.opcode == 0x23) ? 1 : 0;
        // * ALU_src
    if(IFID[1].inst.opcode == 0x8 || IFID[1].inst.opcode == 0x9 || IFID[1].inst.opcode == 0xa || IFID[1].inst.opcode == 0xb || IFID[1].inst.opcode == 0x23 || IFID[1].inst.opcode == 0x2b) IDEX[0].ALU_src = 1;
    else if(IFID[1].inst.opcode == 0xc || IFID[1].inst.opcode == 0xd) IDEX[0].ALU_src = 2;
    else IDEX[0].ALU_src = 0;
        // * ALU_op
    if(IFID[1].inst.opcode == 0x8 || IFID[1].inst.opcode == 0x9 || IFID[1].inst.opcode == 0x23 || IFID[1].inst.opcode == 0x2b || ((IFID[1].inst.opcode == 0x0) && ((IFID[1].inst.func == 0x20) || (IFID[1].inst.func == 0x21)))) IDEX[0].ALU_op = '+';
    if(IFID[1].inst.opcode == 0x0 && ((IFID[1].inst.func == 0x22) || (IFID[1].inst.func == 0x23))) IDEX[0].ALU_op = '-';
    if(IFID[1].inst.opcode == 0xc || ((IFID[1].inst.opcode == 0x0) && (IFID[1].inst.func == 0x24))) IDEX[0].ALU_op = '&';
    if(IFID[1].inst.opcode == 0xd || ((IFID[1].inst.opcode == 0x0) && (IFID[1].inst.func == 0x25))) IDEX[0].ALU_op = '|';
    if(IFID[1].inst.opcode == 0x0 && IFID[1].inst.func == 0x00) IDEX[0].ALU_op = '<';
    if(IFID[1].inst.opcode == 0x0 && IFID[1].inst.func == 0x02) IDEX[0].ALU_op = '>';
    if(IFID[1].inst.opcode == 0x0 && IFID[1].inst.func == 0x27) IDEX[0].ALU_op = '~';
}

void set_target(struct _idex IDEX[], bool *br_cond, unsigned int *br_count, int br_addr, int j_addr, int *target, int ALU_in1, int ALU_in2)
{
    if(IDEX[1].isBeq)
    {
        if (ALU_in1 == ALU_in2)
        {
            printf("branch (equal) taken\n");
            *br_cond = true;
            *br_count += 1;
        }
        else *br_cond = false;
    }
    if(IDEX[1].isBne)
    {
        if (ALU_in1 != ALU_in2)
        {
            printf("branch (not equal) taken\n");
            *br_cond = true;
            *br_count += 1;
        }
        else *br_cond = false;
    }
    *target = select_MUX(((IDEX[1].isBeq || IDEX[1].isBne) && br_cond), 2, IDEX[1].pc4, br_addr);
    *target = select_MUX(IDEX[1].isJump, 2, *target, j_addr);
    *target = select_MUX(IDEX[1].isJr, 2, *target, ALU_in1);
}

void do_IF(struct _ifid IFID[], int mem[], int pc)
{
    IFID[0].pc = pc;
    IFID[0].pc4 = (pc + 4);
    IFID[0].inst.instruction = mem[pc/4];
    // ! Exception for out of bounds memory access
    if((pc / 4) < 0 || ((pc / 4) > MEM_SIZE))
    {
        fprintf(stderr, "EXCEPTION OCCURRED: Instruction memory access out of bounds --> %d\n", (pc/4));
        exit(EXIT_FAILURE);
    }
}

void do_ID(struct _idex IDEX[], struct _ifid IFID[], int *target, unsigned char *i1, unsigned char *i2, int *d1, int *d2)
{
    // * Assign control signals
    assign_control_signals(IDEX, IFID);
    identify_instruction(IDEX, reg, IFID, *target);

    // * Classify instruction type and increment the corresponding instruction type count
    classify_instruction(&j_count, &i_count, &r_count, &mem_count, IFID);
    inst_count++;

    // ! Exception for out of bounds register access
    if(*i1 > REG_COUNT || *i2 > REG_COUNT)
    {
        fprintf(stderr, "EXCEPTION OCCURRED: Register access out of bounds");
        exit(EXIT_FAILURE);
    }

    *i1 = select_MUX(isShift, 2, IFID[1].inst.rs, IFID[1].inst.rt);
    *i2 = IFID[1].inst.rt;
    *d1 = reg[*i1].data;
    *d2 = reg[*i2].data;
    
    IDEX[0].v1 = *d1;
    IDEX[0].v2 = select_MUX(isShift, 2, *d2, IFID[1].inst.shamt);
    IDEX[0].write_reg = select_MUX(reg_dst, 3, IFID[1].inst.rt, IFID[1].inst.rd , 31);
    IDEX[0].address = IFID[1].inst.address;
    IDEX[0].pc = IFID[1].pc;
    IDEX[0].pc4 = IFID[1].pc4;
    IDEX[0].rs = IFID[1].inst.rs;
    IDEX[0].rt = IFID[1].inst.rt;
    IDEX[0].fetch_src = IFID[1].fetch_src;
}

void check_stall(struct _ifid IFID[], struct _idex IDEX[], struct _exmem EXMEM[], struct _memwb MEMWB[], struct _register reg[], bool isShift, int i1, int i2)
{
    // * if read register 1 is rs and it is equal to write_reg of EX/MEM/WB (and reg_write is true)
    if
    (   
        ((i1 == IDEX[1].write_reg) && (!isShift && IFID[1].inst.rs != 0) && IDEX[1].reg_write) ||
        ((i1 == EXMEM[1].write_reg) && (!isShift && IFID[1].inst.rs != 0) && EXMEM[1].reg_write) ||
        ((i1 == MEMWB[1].write_reg) && (!isShift && IFID[1].inst.rs != 0) && MEMWB[1].reg_write)
    ) 
    {
        reg[i1].valid = false;
    }

    // * if read register 2 is rt and it is equal to write_reg of EX/MEM/WB (and reg_write is true)
    if
    (
        ((i2 == IDEX[1].write_reg) && (IFID[1].inst.rt != 0) && IDEX[1].reg_write) ||
        ((i2 == EXMEM[1].write_reg) && (IFID[1].inst.rt != 0) && EXMEM[1].reg_write) ||
        ((i2 == MEMWB[1].write_reg) && (IFID[1].inst.rt != 0) && MEMWB[1].reg_write)
    )
    {
        reg[i2].valid = false;
    }
}

void check_forwarding(struct _forwardingUnit *forwarding_unit, struct _idex IDEX[], struct _exmem EXMEM[], struct _register reg[], int *ALU_in1, int *ALU_in2, int *write_data)
{
    // * If rs (containing a non-zero value) in EX is equal to write_reg of MEM/WB and reg_write is true
    if((forwarding_unit->rs != 0) && (forwarding_unit->rs == forwarding_unit->from_MEM) && forwarding_unit->reg_write_MEM) forwarding_unit->ForwardA = 2;
    else if((forwarding_unit->rs != 0) && (forwarding_unit->rs == forwarding_unit->from_WB) && forwarding_unit->reg_write_WB) forwarding_unit->ForwardA = 1;
    else forwarding_unit->ForwardA = 0;

    // * If rt (containing a non-zero value) in EX is equal to write_reg of MEM/WB, reg_write is true, and rt's value is going to be used
    if((forwarding_unit->rt != 0) && (forwarding_unit->rt == forwarding_unit->from_MEM) && forwarding_unit->reg_write_MEM && forwarding_unit->ALU_src == 0) forwarding_unit->ForwardB = 2;
    else if((forwarding_unit->rt != 0) && (forwarding_unit->rt == forwarding_unit->from_WB) && forwarding_unit->reg_write_WB && forwarding_unit->ALU_src == 0) forwarding_unit->ForwardB = 1;
    else forwarding_unit->ForwardB = 0;

    // * If rt's value (v2) is required for a SW instruction's write_data and rt (containing a non-zero value) in EX is equal to write_reg of MEM/WB
    if((forwarding_unit->rt != 0) && (forwarding_unit->rt == forwarding_unit->from_MEM) && forwarding_unit->reg_write_MEM && forwarding_unit->mem_write) forwarding_unit->ForwardC = 2;
    else if((forwarding_unit->rt != 0) && (forwarding_unit->rt == forwarding_unit->from_WB) && forwarding_unit->reg_write_WB && forwarding_unit->mem_write) forwarding_unit->ForwardC = 1;
    else forwarding_unit->ForwardC = 0;

    // * Set the value to be forwarded from MEM stage
    int value_from_mem = select_MUX(EXMEM[1].isSet, 2, EXMEM[1].ALU_out, EXMEM[1].set_cond);
    value_from_mem = select_MUX(EXMEM[1].isLui, 2, value_from_mem, EXMEM[1].ext_imm);

    // * If rs is equal to write_reg of WB, refresh the value of v1 by re-reading from the register after WB is finished (read updated value)
    if(forwarding_unit->i1 == forwarding_unit->from_WB) IDEX[0].v1 = reg[i1].data;

    // * Set the first ALU input by fetching the value from the correspponding stage
    *ALU_in1 = select_MUX(forwarding_unit->ForwardA, 3, IDEX[1].v1, *write_data, value_from_mem);

    // * Set the second ALU input by fetching the value from the corresponding stage
    *ALU_in2 = select_MUX(forwarding_unit->ForwardB, 3, forwarding_unit->src_input, *write_data, value_from_mem);

    // * Set the memory write_data by fetching the value from the corresponding stage
    EXMEM[0].v2 = select_MUX(forwarding_unit->ForwardC, 3, IDEX[1].v2, *write_data, value_from_mem);

    // * Calculate the output of ALU
    EXMEM[0].ALU_out = set_ALU_op(IDEX[1].ALU_op, *ALU_in1, *ALU_in2);

    // * Determine the output of ALU in set instructions (equal)
    if(EXMEM[0].isSet) EXMEM[0].set_cond = (*ALU_in1 < *ALU_in2) ? 1 : 0;
}

void do_EX(struct _exmem EXMEM[], struct _idex IDEX[], struct _memwb MEMWB[], struct _ifid IFID[], int *src_input, int *j_addr, int *br_addr)
{
    // * Determine src_input
    *src_input = select_MUX(IDEX[1].ALU_src, 3, IDEX[1].v2, IDEX[1].s_imm, IDEX[1].ext_imm);

    // * Pass values (no calculations required)
    EXMEM[0].pc4 = IDEX[1].pc4;
    EXMEM[0].v2 = IDEX[1].v2;
    EXMEM[0].write_reg = IDEX[1].write_reg;
    EXMEM[0].ext_imm = IDEX[1].ext_imm;
    EXMEM[0].isJal = IDEX[1].isJal;
    EXMEM[0].isSet = IDEX[1].isSet;
    EXMEM[0].isLui = IDEX[1].isLui;
    EXMEM[0].reg_write = IDEX[1].reg_write;
    EXMEM[0].mem_write = IDEX[1].mem_write;
    EXMEM[0].mem_to_reg = IDEX[1].mem_to_reg;

    // * Calculate jump and branch address
    *j_addr = (((IFID[1].pc4 & 0xf0000000) >> 28) | (IDEX[1].address << 2));
    *br_addr = IDEX[1].pc4 + (IDEX[1].s_imm << 2);
}

void do_MEM(struct _memwb MEMWB[], struct _exmem EXMEM[], int mem[], struct _forwardingUnit *forwarding_unit)
{
    // * Pass values (no calculations required)
    MEMWB[0].pc4 = EXMEM[1].pc4;
    MEMWB[0].write_reg = EXMEM[1].write_reg;
    MEMWB[0].ext_imm = EXMEM[1].ext_imm;
    MEMWB[0].set_cond = EXMEM[1].set_cond;
    MEMWB[0].ALU_out = EXMEM[1].ALU_out;
    MEMWB[0].isJal = EXMEM[1].isJal;
    MEMWB[0].isSet = EXMEM[1].isSet;
    MEMWB[0].isLui = EXMEM[1].isLui;
    MEMWB[0].reg_write = EXMEM[1].reg_write;
    MEMWB[0].mem_to_reg = EXMEM[1].mem_to_reg;

    // * Read the value of a certain memory index if mem_to_reg or mem_read is true
    if(EXMEM[1].mem_to_reg || EXMEM[1].mem_read) MEMWB[0].mem_out = mem[EXMEM[1].ALU_out/4];

    // * Write in the designated memory index if mem_write is true
    if(EXMEM[1].mem_write) mem[EXMEM[1].ALU_out/4] = EXMEM[1].v2;
}

void do_WB(int *pc8, int *write_data, struct _memwb MEMWB[], int mem[], struct _register reg[], struct _forwardingUnit *forwarding_unit)
{
    // * Set PC + 8 for JAL instruction
    *pc8 = MEMWB[1].pc4 + 4;

    // * Set write_data depending on the used control signals
    *write_data = select_MUX(MEMWB[1].mem_to_reg, 2, MEMWB[1].ALU_out, MEMWB[1].mem_out);
    *write_data = select_MUX(MEMWB[1].isJal, 2, *write_data, *pc8);
    *write_data = select_MUX(MEMWB[1].isSet, 2, *write_data, MEMWB[1].set_cond);
    *write_data = select_MUX(MEMWB[1].isLui, 2, *write_data, MEMWB[1].ext_imm);

    // * Update register write_reg if reg_write is true
    if(MEMWB[1].reg_write)
    {
        reg[MEMWB[1].write_reg].data = *write_data;
        reg[MEMWB[1].write_reg].valid = true;
    }
}

unsigned char update_counter(unsigned char counter, bool br_cond)
{
    if(br_cond) 
    {
        // * Increment if less than "Strongly Taken" (3 in decimal)
        if(counter < 3) counter++;
    }
    // * Decrement if more than "Strongly Not Taken" (0 in decimal)
    else if(counter > 0) counter--;
    return counter;
}

// Simulate the branch prediction and updating mechanism
void predict_branch(unsigned char *BHR, unsigned char PHT[], bool br_cond)
{
    // * Use the BHR as an index for PHT
    int index = *BHR;

    // * Predict the branch condition
    bool predicted_outcome = PHT[index] >= 2;  // * 2 and 3 are considered taken, 0 and 1 not taken

    // * Print the predicted and the actual branch condition
    // printf("Predicted: %s, Actual: %s\n", predicted_outcome ? "Taken" : "Not Taken", br_cond ? "Taken" : "Not Taken");

    // * Update the corresponding PHT entry based on the actual branch outcome
    PHT[index] = update_counter(PHT[index], br_cond);

    // * Update the BHR (shift left and add the new outcome)
    *BHR = ((*BHR << 1) | br_cond) & 3;  // * Keep BHR to 2 bits
}

void write_BTB(struct _BTB *BTB_entry, int pc, int target, bool HB)
{
    // * Write the values as an entry in the BTB
    BTB_entry->pc = pc;
    BTB_entry->target = target;
    BTB_entry->HB = HB;
}

bool check_BTB(int pc, struct _BTB BTB[], unsigned int BTB_last_entry, unsigned int *index, bool *notFound)
{
    // * Check if the current pc is listed in any of the BTB entries
    for(int i = 0; i < BTB_last_entry; i++)
    {
        if(pc == BTB[i].pc)
        {
            *notFound = false;
            *index = i; // * Save the matching index
            return BTB[i].HB; // * Return true if HB of the matching pc is true
        }
    }
    *notFound = true; // * Indicate that a match is not found
    return false;
}

void flushIF(struct _ifid IFID[])
{
    for(int i = 0; i < 2; i++)
    {
        IFID[i].pc = 0;
        IFID[i].pc4 = 0;
        IFID[i].inst.instruction = 0;
        IFID[i].valid = 0;
        // IFID[i].fetch_src = 0;
    }
}

void flushID(struct _idex IDEX[], unsigned char *i1, unsigned char *i2, int *d1, int *d2)
{
    *i1 = 0;
    *i2 = 0;
    *d1 = 0;
    *d2 = 0;

    for(int i = 0; i < 2; i++)
    {
        IDEX[i].address = 0;
        IDEX[i].pc = 0;
        IDEX[i].pc4 = 0;
        IDEX[i].v1 = 0;
        IDEX[i].v2 = 0;
        IDEX[i].s_imm = 0;
        IDEX[i].write_reg = 0;
        IDEX[i].ext_imm = 0;
        IDEX[i].isJr = 0;
        IDEX[i].isJump = 0;
        IDEX[i].isBeq = 0;
        IDEX[i].isBne = 0;
        IDEX[i].isJal = 0;
        IDEX[i].isSet = 0;
        IDEX[i].isLui = 0;
        IDEX[i].reg_write = 0;
        IDEX[i].mem_write = 0;
        IDEX[i].mem_read = 0;
        IDEX[i].mem_to_reg = 0;
        IDEX[i].ALU_src = 0;
        IDEX[i].ALU_op = '<';
        IDEX[i].rs = 0;
        IDEX[i].rt = 0;
        IDEX[i].valid = 0;
        // IDEX[i].fetch_src = 0;
    }
}