#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "variables.h"
#include "functions.h"
#include "instruction.h"
#define MEM_SIZE 0x400000

// * Initialize a 16 MB memory, a program counter, 32 registers with 0 values, and an instruction
int mem[MEM_SIZE];
int pc = 0;
int reg[32] = {0, };
struct _instruction inst;

// * Instruction counts
unsigned int inst_count, r_count, i_count, j_count, mem_count, br_count = 0;

// * values
int write_reg, write_data, ALU_input, mem_out, target, ALU_out = 0;
bool br_cond, set_cond = false;

int main(int argc, char *argv[])
{
    FILE *fp;
    fp = fopen(argv[1], "rb");

    FILE *fptr;
    fptr = fopen("log.txt", "w");

    int i = 0;
    int var = 0;

    // TODO: Initialization
    // ! handle exception for missing file
    if (fp == NULL)
    {
        fprintf(stderr, "EXCEPTION OCCURED: File '%s' not found\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        if(i > MEM_SIZE)
        {
            fprintf(stderr, "EXCEPTION OCCURED: Memory out of bounds\n");
            exit(EXIT_FAILURE);
        }

        int ret = fread(&var, sizeof(var), 1, fp);
        if(ret == 0) break;

        // ! handle exception for invalid instruction
        if(sizeof(ret) != sizeof(int))
        {
            fprintf(stderr, "EXCEPTION OCCURED: Invalid instruction\n");
            exit(EXIT_FAILURE);
        }
        printf("fread binary: 0x%x\n", var);

        unsigned int b1, b2, b3, b4;
        b1 = var & 0x000000ff;
        b2 = var & 0x0000ff00;
        b3 = var & 0x00ff0000;
        b4 = (var >> 24) & 0xff;
        // printf("b1: 0x%x\tb2: 0x%x\tb3: 0x%x\tb4: 0x%x\n", b1, b2, b3, b4);
        int res = (b1 << 24) | (b2 << 8) | (b3 >> 8) | b4;

        // printf("reordered data: 0x%x\n", res);

        inst.instruction = res;
        mem[i] = inst.instruction;
        i++;
        // printf("inst.opcode: %x\n\n", (inst.instruction >> 26) & 0x3f);
        // printf("inst.func: 0x%x\n\n)", inst.instruction & 0x3f);
    } 
    fclose(fp);

    // * Initialize SP(Reg[29]) to stack's starting point and RA (Reg[31]) to -1 (end of execution)
    reg[29] = 0x1000000;
    reg[31] = 0xffffffff;

    while(pc != 0xffffffff)
    {
        // TODO: Instruction Fetch
        // ! handling exception for out of bounds memory access
        if((pc / 4) < 0 || ((pc / 4) > MEM_SIZE))
        {
            fprintf(stderr, "EXCEPTION OCCURED: Memory access out of bounds\n");
            exit(EXIT_FAILURE);
        }
        inst.instruction = mem[pc/4];

        // TODO: Instruction Decode
        inst.opcode = (inst.instruction >> 26) & 0x3f;
        inst.rs = (inst.instruction >> 21) & 0x1f;
        inst.rt = (inst.instruction >> 16) & 0x1f;
        inst.rd = (inst.instruction >> 11) & 0x1f;
        inst.shamt = (inst.instruction >> 6) & 0x1f;
        inst.func = inst.instruction & 0x3f;
        inst.imm = inst.instruction & 0xffff;
        inst.s_imm = (inst.imm & 0x8000) ? (inst.imm | 0xffff0000) : (inst.imm);
        inst.ext_imm = 0;

        // ! handling exception for out of bounds register access
        if(inst.rs < 0 || inst.rt < 0 || inst.rd < 0 || inst.rs > 31 || inst.rt > 31 || inst.rd > 31)
        {
            fprintf(stderr, "EXCEPTION OCCURED: Register access out of bounds\n");
            exit(EXIT_FAILURE);
        }

        // TODO: Execute/ALU op
        assign_control_signals(&inst);
        check_conditions(&br_cond, &set_cond, reg, &inst);
        if(br_cond) br_count++;
        classify_instruction(&j_count, &i_count, &r_count, &mem_count, &inst);
        
        switch(inst.opcode)
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
                printf("\n~~~\nOPCODE: BEQ\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.s_imm);
                break;

            // * BNE (I-type)
            case 0x5:
                printf("\n~~~\nOPCODE: BNE\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.s_imm);
                break;

            // * ADDI (I-type)
            case 0x8:
                printf("\n~~~\nOPCODE: ADDI\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.s_imm);
                break;

            // * ADDIU (I-type)
            case 0x9:
                printf("\n~~~\nOPCODE: ADDIU\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.imm);
                break;

            // * SLTI (I-type)
            case 0xa:
                printf("\n~~~\nOPCODE: SLTI\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.s_imm);
                break;

            // * SLTIU (I-type)
            case 0xb:
                printf("\n~~~\nOPCODE: SLTIU\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.s_imm);
                break;
            
            // * ANDI
            case 0xc:
                printf("\n~~~\nOPCODE: ANDI\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.ext_imm);
                break;

            // * ORI
            case 0xd:
                printf("\n~~~\nOPCODE: ORI\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.ext_imm);
                break;

            // * LUI
            case 0xf:
                printf("\n~~~\nOPCODE: LUI\n\nRT: %d\nIMM: %d\n~~~\n", inst.rt, inst.ext_imm);
                break;

            // * LW (I-type)
            case 0x23:
                printf("\n~~~\nOPCODE: LW\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.s_imm);
                break;

            // * SW (I-type)
            case 0x2b:
                printf("\n~~~\nOPCODE: SW\nRS: %d\nRT: %d\nIMM: %d\n~~~\n", inst.rs, inst.rt, inst.s_imm);
                break;

            // * R-type
            case 0x0:
                switch(inst.func)
                {
                    // * SLL
                    case 0x00:
                        printf("\n~~~\nOPCODE: SLL\nRT: %d\nRD: %d\nSHAMT: %d\n~~~\n", inst.rt, inst.rd, inst.shamt);
                        break;

                    // * SRL
                    case 0x02:
                        printf("\n~~~\nOPCODE: SRL\nRT: %d\nRD: %d\nSHAMT: %d\n~~~\n", inst.rt, inst.rd, inst.shamt);
                        break;

                    // * JR
                    case 0x08:
                        printf("\n~~~\nOPCODE: JR\nADDRESS: 0x%x\n~~~\n", reg[inst.rs]);
                        break;

                    // * JALR
                    case 0x09:
                        printf("\n~~~\nOPCODE: JALR\nADDRESS: 0x%x\n~~~\n", reg[inst.rs]);
                        break;

                    // * ADD
                    case 0x20:
                        printf("\n~~~\nOPCODE: ADD\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    // * ADDU
                    case 0x21:
                        printf("\n~~~\nOPCODE: ADDU\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    // * SUB
                    case 0x22:
                        printf("\n~~~\nOPCODE: SUB\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    // * SUBU
                    case 0x23:
                        printf("\n~~~\nOPCODE: ADDU\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    // * AND
                    case 0x24:
                        printf("\n~~~\nOPCODE: AND\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    // * OR
                    case 0x25:
                        printf("\n~~~\nOPCODE: OR\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    // * NOR
                    case 0x27:
                        printf("\n~~~\nOPCODE: NOR\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    // * SLT
                    case 0x2a:
                        printf("\n~~~\nOPCODE: SLT\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    // * SLTU
                    case 0x2b:
                        printf("\n~~~\nOPCODE: SLTU\nRS: %d\nRT: %d\nRD: %d\n~~~\n", inst.rs, inst.rt, inst.rd);
                        break;

                    default:
                        // ! Exception for unknown command
                        fprintf(stderr, "EXCEPTION OCCURED: Unknown command\nopcode = 0x%x\nfunc = 0x%x", inst.opcode, inst.func);
                        exit(EXIT_FAILURE);
                        break;
                }
                break;
            
            default:
                // ! Exception for unknown command
                fprintf(stderr, "EXCEPTION OCCURED: Unknown command\n");
                exit(EXIT_FAILURE);
                break;
        }
        inst_count++;

        write_reg = select_MUX(inst.reg_dst, 3, inst.rt, inst.rd , 31);

        ALU_input = select_MUX(inst.ALU_src, 3, reg[inst.rt], inst.s_imm, inst.ext_imm);

        if(inst.isShift) ALU_out = set_ALU_op(inst.ALU_op, reg[inst.rt], inst.shamt);
        else ALU_out = set_ALU_op(inst.ALU_op, reg[inst.rs], ALU_input);

        if(inst.mem_to_reg) mem_out = mem[ALU_out/4];

        write_data = select_MUX(inst.mem_to_reg, 2, ALU_out, mem_out);
        write_data = select_MUX(inst.isJal, 2, write_data, (pc + 8));
        write_data = select_MUX(inst.isSet, 2, write_data, set_cond);
        write_data = select_MUX(inst.isLui, 2, write_data, inst.ext_imm);
    
        target = select_MUX((inst.isBranch & br_cond), 2, (pc + 4), ((pc + 4) + (inst.s_imm << 2)));
        target = select_MUX(inst.isJump, 2, target, ((((pc + 4) & 0xf0000000) >> 28) | ((inst.instruction & 0x3ffffff) << 2)));
        target = select_MUX(inst.isJr, 2, target, reg[inst.rs]);

        // ! handling exceptions for out of bounds memory access
        if((target > MEM_SIZE) & (target != 0xffffffff))
        {
            fprintf(stderr, "EXCEPTION OCCURED: Memory access out of bounds\n");
            exit(EXIT_FAILURE);
        }
        pc = target;

        // TODO: Memory Access (Load/Store)
        // ! Exception for out of bounds memory access (SW and LW)
        if(inst.mem_write == 1 || inst.mem_to_reg == 1)
        {
            if((ALU_out / 4) < 0 || ((ALU_out / 4) > MEM_SIZE))
            {
                fprintf(stderr, "EXCEPTION OCCURED: Memory access out of bounds\n");
                exit(EXIT_FAILURE);
            }
        }
        if(inst.mem_write) mem[ALU_out/4] = reg[inst.rt];

        // TODO: Write Back Result to Registers
        if(inst.reg_write) reg[write_reg] = write_data;

        // TODO: Output Result & Stats
        if(inst.mem_write) printf("CHANGE OCCURED: Memory[%d] = %d\n", (ALU_out/4), mem[ALU_out/4]);
        if(inst.reg_write) printf("CHANGE OCCURED: Register[%d] = %d\n", write_reg, reg[write_reg]);
        if(inst.isJump || inst.isJal || inst.isJr || (inst.isBranch & br_cond)) printf("Jumped to 0x%x\n", target);
        show_stats();

        // ? Debug (log file)
        #ifdef DEBUG_PC
        if(inst.opcode == 0x2 || inst.opcode == 0x3) fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nADDRESS: 0x%x\nPC: 0x%x\n\n", inst_count, inst.opcode, (inst.instruction & 0x3ffffff), pc);
        else if(!inst.opcode) fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nFUNC = 0x%x\nRS: %d\nRT: %d\nRD: %d\nSHAMT: %d\nPC: 0x%x\n\n",inst_count, inst.opcode, inst.func, inst.rs, inst.rt, inst.rd, inst.shamt, pc);
        else fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nRS: %d\nRT: %d\nIMM: 0x%x\nPC: 0x%x\n\n",inst_count, inst.opcode, inst.rs, inst.rt, inst.imm, pc);
        #endif
    }
    fclose(fptr);
    return 0;
}