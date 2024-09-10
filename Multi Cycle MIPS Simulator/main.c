#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "variables.h"
#include "functions.h"
#include "struct.h"
#define MEM_SIZE 0x400000
#define REG_COUNT 32
#define MAX_BTB_ENTRY 10000
#define OUTCOME_COUNT 10
#define PHT_COUNT 4

// * Initialize a 16 MB memory, 32 registers, a program counter, and an instruction
int mem[MEM_SIZE];
int pc = 0;
struct _register reg[REG_COUNT];
struct _instruction inst;
struct _forwardingUnit forwarding_unit;
struct _BTB BTB[MAX_BTB_ENTRY];

// * Instruction count
unsigned int inst_count, r_count, i_count, j_count, mem_count, br_count, BTB_last_entry, match_index = 0;
unsigned char cycles_after_stall = 0;
int excess_cycles = 4;
int cycle_count = 0;
bool halt_IF, halt_ID, halt_EX, halt_MEM, notFound = false;
// unsigned char instIndex = 0;

// * Control signals that do not need to be passed between latches
unsigned char reg_dst, ForwardA, ForwardB, fetch_src = 0;
bool isShift, isUnsigned = false;

// * Values that do not need to be passed between latches
int write_data, src_input, ALU_in1, ALU_in2, target, fetch_addr, j_addr, br_addr, pc8, d1, d2 = 0;
bool br_cond, stall = false;
unsigned char set_cond, i1, i2 = 0;

// * Latches
struct _ifid IFID[2];
struct _idex IDEX[2];
struct _exmem EXMEM[2];
struct _memwb MEMWB[2];

// * Branch predicting instruments (BHR and PHT)
unsigned int BHR;
unsigned char PHT[PHT_COUNT];
bool branch_outcomes[OUTCOME_COUNT];
bool BTB_out;

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
        fprintf(stderr, "EXCEPTION OCCURRED: File '%s' not found\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        // ! Exception for out of bounds memory access
        if(i > MEM_SIZE)
        {
            fprintf(stderr, "EXCEPTION OCCURRED: Memory out of bounds\n");
            exit(EXIT_FAILURE);
        }

        int ret = fread(&var, sizeof(var), 1, fp);
        if(ret == 0) break;

        // ! Exception for invalid instruction
        if(sizeof(ret) != sizeof(int))
        {
            fprintf(stderr, "EXCEPTION OCCURRED: Invalid instruction\n");
            exit(EXIT_FAILURE);
        }

        // * Reorder endian
        unsigned int b1, b2, b3, b4;
        b1 = var & 0x000000ff;
        b2 = var & 0x0000ff00;
        b3 = var & 0x00ff0000;
        b4 = (var >> 24) & 0xff;
        int res = (b1 << 24) | (b2 << 8) | (b3 >> 8) | b4;

        inst.instruction = res;
        mem[i] = inst.instruction;
        i++;
    } 
    fclose(fp);

    // * Initialize SP(Reg[29]) to stack's starting point and RA (Reg[31]) to -1 (end of execution), the rest to 0
    initialize_registers(reg);

    // * Initialize branch predicting instruments (BHR and PHT)
    initialize_branch_predictor(&BHR, PHT, branch_outcomes);

    // * Set initial state of instruction to true
    IFID[0].valid = true;

    while(1)
    {
        cycle_count++;
        /**/printf("CYCLE %d\n", cycle_count);
        /**/fprintf(fptr, "CYCLE %d\n", cycle_count);
        /**/fprintf(fptr, "PC: 0x%x\n", pc);
        /**/showStats();
        /**/fprintf(fptr, "Displaying all registers:\n");
        /**/for(int i = 0; i < 8; i++)
        /**/{
        /**/    for(int j = 0; j <= 24 ; j+=8)
        /**/    {
        /**/        fprintf(fptr, "reg[%02d]: %-8d 0x%-16x", (i + j), reg[i + j].data, reg[i + j].data);
        /**/    }
        /**/    fprintf(fptr, "\n");
        /**/}
        
        /**/for(int i = 0; i < REG_COUNT; i++)
        /**/{
        /**/    if(!reg[i].valid) fprintf(fptr, "***Register [%d] is invalid***\n", i);
        /**/}

        if(!stall)
        {
            cycles_after_stall = 0;
            // * Checks if there are more instruction to fetch
            if(!halt_IF)
            {
                // TODO: Instruction Fetch
                // instIndex = (pc / 4) + 1;
                // ! Exception for out of bounds memory access
                if((pc / 4) < 0 || ((pc / 4) > MEM_SIZE))
                {
                    fprintf(stderr, "EXCEPTION OCCURRED: Instruction memory access out of bounds --> PC is trying to access instruction memory index %d\n", (pc/4));
                    exit(EXIT_FAILURE);
                }
                
                if(pc != 0xffffffff)
                {
                    /**/fprintf(fptr, "\nStatus: Performing IF...\n");
                    do_IF(IFID, mem, pc);
                    /**/fprintf(fptr, "Status: IF completed.\n");
                    /**/fprintf(fptr, "Status: Predicting branch...\n");

                    // * Predict branch based on the PHT entry pointed by BHR
                    for(int i = 0; i < OUTCOME_COUNT; i++) predict_branch(&BHR, PHT, branch_outcomes[i]);

                    // * Check the previous history of this pc (whether or not it is a branch, whether or not it is taken, target information)
                    BTB_out = check_BTB(pc, BTB, BTB_last_entry, &match_index, &notFound);

                    // * Determine the source of fetch address (taken branch/jump, or non taken branch/no branch)
                    fetch_src = BTB_out & PHT[BHR];

                    // * Record the value of fetch_src to be examined later in EX stage
                    IFID[0].fetch_src = (fetch_src) ? true : false;

                    // * Fetch the next address
                    fetch_addr = select_MUX(fetch_src, 2, (pc + 4), BTB[match_index].target);
                    pc = fetch_addr;

                    if(fetch_src == 1)
                    {
                        /**/fprintf(fptr, "\tBranch taken. Jumping to 0x%x\n", pc);
                    }

                    if(cycle_count == 1) goto endCycle;
                }
            }

            if(!halt_ID)
            {
                // TODO: Instruction Decode
                /**/fprintf(fptr, "\nStatus: Performing decode...\n");
                decode_instruction(IDEX, IFID);
                /**/fprintf(fptr, "Status: Decode completed: \n");

                /**/if(IFID[1].inst.opcode == 0x2 || IFID[1].inst.opcode == 0x3)
                /**/{
                /**/    fprintf(fptr, "\tInstruction: %08x\n\tOPCODE: 0x%x\n\tADDRESS: 0x%x\n\tPC: 0x%x\n", IFID[1].inst.instruction, IFID[1].inst.opcode, IFID[1].inst.address, IFID[1].pc);
                /**/}
                /**/else if(IFID[1].inst.opcode == 0x0)
                /**/{
                /**/    fprintf(fptr, "\tInstruction: %08x\n\tOPCODE: 0x%x\n\tFUNC: 0x%x\n\tRS: %d\n\tRT: %d\n\tRD: %d\n\tSHAMT: 0x%x\n\tPC: 0x%x\n",IFID[1].inst.instruction, IFID[1].inst.opcode, IFID[1].inst.func, IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd, IFID[1].inst.shamt, IFID[1].pc);
                /**/}
                /**/else fprintf(fptr, "\tInstruction: %08x\n\tOPCODE: 0x%x\n\tRS: %d\n\tRT: %d\n\tIMM: 0x%x\n\tPC: 0x%x\n",IFID[1].inst.instruction, IFID[1].inst.opcode, IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.imm_, IFID[1].pc);

                // ! Exception for out of bounds register access
                if(IFID[1].inst.rs < 0 || IFID[1].inst.rt < 0 || IFID[1].inst.rd < 0 || IFID[1].inst.rs > 31 || IFID[1].inst.rt > 31 || IFID[1].inst.rd > 31)
                {
                    fprintf(stderr, "EXCEPTION OCCURRED: Register access out of bounds\n");
                    exit(EXIT_FAILURE);
                }

                /**/fprintf(fptr, "Status: Performing ID...\n");
                do_ID(IDEX, IFID, &target, &i1, &i2, &d1, &d2);
                /**/fprintf(fptr, "Status: ID completed.\n");

                // * Check if any registers that will be used are invalid, if so, the instruction should be invalidated
                if(!reg[i1].valid || !reg[i2].valid || !reg[IDEX[0].write_reg].valid) IDEX[0].valid = false;

                // ? Debug (log file)
                #ifdef DEBUG_PC
                if(IFID[1].inst.opcode == 0x2 || IFID[1].inst.opcode == 0x3) fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nADDRESS: 0x%x\nPC: 0x%x\n\n", inst_count, IFID[1].inst.opcode, (IFID[1].inst.inst & 0x3ffffff), pc);
                else if(IFID[1].inst.opcode == 0x0) fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nFUNC = 0x%x\nRS: %d\nRT: %d\nRD: %d\nSHAMT: %d\nPC: 0x%x\n\n",inst_count, IFID[1].inst.opcode, IFID[1].inst.func, IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.rd, IFID[1].inst.shamt, pc);
                else fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nRS: %d\nRT: %d\nIMM: 0x%x\nPC: 0x%x\n\n",inst_count, IFID[1].inst.opcode, IFID[1].inst.rs, IFID[1].inst.rt, IFID[1].inst.imm_, IFID[1].inst.pc);
                #endif

                // * Check the necessity for stalling
                // check_stall(IFID, IDEX, EXMEM, MEMWB, reg, isShift, i1, i2);

                if(!reg[i1].valid || !reg[i2].valid) stall = true;
                if(cycle_count == 2) goto endCycle;
            }
        }
        else cycles_after_stall++;

        if(!halt_EX)
        {
            // TODO: Execute/ALU op
            /**/fprintf(fptr, "\nStatus: Performing EX...\n");
            do_EX(EXMEM, IDEX, MEMWB, IFID, &src_input, &j_addr, &br_addr);
            /**/fprintf(fptr, "Status: EX completed.\n");

            // * If the instruction is a jump instruction, make br_cond true, otherwise false
            if(IDEX[1].isJal || IDEX[1].isJr || IDEX[1].isJump) br_cond = true;
            else br_cond = false;
        }

        if(!halt_MEM)
        {
            // TODO: Memory Access (Load/Store)
            /**/fprintf(fptr, "\nStatus: Performing MEM...\n");
            // ! Exception for out of bounds memory access (SW and LW)
            if(EXMEM[1].mem_write || EXMEM[1].mem_to_reg)
            {
                if((EXMEM[1].ALU_out/4) < 0 || ((EXMEM[1].ALU_out/4) > MEM_SIZE))
                {
                    fprintf(stderr, "EXCEPTION OCCURRED: Memory access out of bounds --> %d\n", (EXMEM[1].ALU_out/4));
                    exit(EXIT_FAILURE);
                }
            }
            do_MEM(MEMWB, EXMEM, mem, &forwarding_unit);
            /**/fprintf(fptr, "Status: MEM completed.\n\n");

            if(EXMEM[1].mem_write)
            {
                printf("CHANGE OCCURRED: Memory[%d] = %d\n\n", (EXMEM[1].ALU_out/4), mem[EXMEM[1].ALU_out/4]);
                fprintf(fptr, "CHANGE OCCURRED: Memory[%d] = %d\n\n", (EXMEM[1].ALU_out/4), mem[EXMEM[1].ALU_out/4]);
            }
            if(cycle_count == 4) goto endCycle;
        }

        // TODO: Write Back Result to Registers
        /**/fprintf(fptr, "Status: Performing WB...\n");
        do_WB(&pc8, &write_data, MEMWB, mem, reg, &forwarding_unit);
        /**/fprintf(fptr, "Status: WB completed.\n\n");

        // TODO: Output Result & Stats (Memory & Register Update)
        if(MEMWB[1].reg_write) 
        {
            printf("CHANGE OCCURRED: Register[%d] = %d\n\n", MEMWB[1].write_reg, reg[MEMWB[1].write_reg].data);
            fprintf(fptr, "CHANGE OCCURRED: Register[%d] = %d\n\n", MEMWB[1].write_reg, reg[MEMWB[1].write_reg].data);
            if(!reg[MEMWB[1].write_reg].valid) reg[MEMWB[1].write_reg].valid = true;
        }

        endCycle:
            if(cycle_count > 2)
            {
                // * Pass values to be evaluated to the forwarding unit
                forwarding_unit.rs = IDEX[1].rs;
                forwarding_unit.rt = IDEX[1].rt;

                forwarding_unit.reg_write_MEM = EXMEM[1].reg_write;
                forwarding_unit.from_MEM = EXMEM[1].write_reg;

                forwarding_unit.reg_write_WB = MEMWB[1].reg_write;
                forwarding_unit.from_WB = MEMWB[1].write_reg;

                forwarding_unit.i1 = i1;
                forwarding_unit.mem_write = IDEX[1].mem_write;
                forwarding_unit.ALU_src = IDEX[1].ALU_src;
                forwarding_unit.src_input = src_input;

                check_forwarding(&forwarding_unit, IDEX, EXMEM, reg, &ALU_in1, &ALU_in2, &write_data);
                if(EXMEM[0].isSet) set_target(IDEX, &br_cond, &br_count, br_addr, j_addr, &target, ALU_in1, ALU_in2);

                // * Set target for jump and branch instructions
                /**/fprintf(fptr, "Status: Setting target...\n");
                set_target(IDEX, &br_cond, &br_count, br_addr, j_addr, &target, ALU_in1, ALU_in2);
                /**/fprintf(fptr, "Status: Target set to 0x%x\n", target);

                // ! Exceptions for out of bounds memory access
                if((IDEX[1].isJump || IDEX[1].isBeq || IDEX[1].isBne) && (target > MEM_SIZE) && (target != 0xffffffff))
                {
                    fprintf(stderr, "EXCEPTION OCCURRED: Jump prompts memory access out of bounds --> %d\n", target);
                    exit(EXIT_FAILURE);
                }

                // * If the instruction is a jump instruction or a taken branch, write the current pc to be listed to the BTB
                if(br_cond) 
                {
                    // ! Exceptions for out of bounds BRB entry
                    if(BTB_last_entry == MAX_BTB_ENTRY) fprintf(stderr, "EXCEPTION OCCURRED: BTB entries have exceeded the maximum limit (255)\n");

                    // * Check if there has been an instance of the corresponding pc enlisted in the BTB
                    check_BTB(IDEX[1].pc, BTB, BTB_last_entry, &match_index, &notFound);
                    if(!notFound)
                    {
                        // * Treat as a mispredicted branch if the target of an enlisted pc value is rewritten
                        if(BTB[match_index].target != target)
                        {
                            pc = IDEX[1].pc;
                            flushIF(IFID);
                            /**/fprintf(fptr, "Status: IF flushed.\n");
                            flushID(IDEX, &i1, &i2, &d1, &d2);
                            /**/fprintf(fptr, "Status: ID flushed.\n");
                        }

                        // * Rewrite values if the corresponding pc has already been listed before as an entry of BTB
                        write_BTB(&BTB[match_index], IDEX[1].pc, target, br_cond);
                        /**/printf("\tBTB Updated: BTB[%d]\n\tPC: 0x%x\n\tTarget: 0x%x\n\tHB: %s\n", match_index, IDEX[1].pc, target, br_cond ? "True" : "False");
                        /**/fprintf(fptr, "\tBTB Updated: BTB[%d]\n\tPC: 0x%x\n\tTarget: 0x%x\n\tHB: %s\n", i, IDEX[1].pc, target, br_cond ? "True" : "False");
                    }
                    else
                    {
                        // * Write the new pc and its values as the latest entry
                        write_BTB(&BTB[BTB_last_entry], IDEX[1].pc, target, br_cond);
                        /**/printf("\tBTB Updated: BTB[%d]\n\tPC: 0x%x\n\tTarget: 0x%x\n\tHB: %s\n", BTB_last_entry, IDEX[1].pc, target, br_cond ? "True" : "False");
                        /**/fprintf(fptr, "\tBTB Updated: BTB[%d]\n\tPC: 0x%x\n\tTarget: 0x%x\n\tHB: %s\n", BTB_last_entry, IDEX[1].pc, target, br_cond ? "True" : "False");
                        BTB_last_entry += 1;
                    }
                }

                // * If the instruction is an exit branch (not taken branch), set PC to PC + 4 and invalidate instructions in ID and IF (from mispredicted branch loop)
                if(!br_cond && !fetch_src && (IDEX[1].isBeq || IDEX[1].isBne))
                {
                    pc = IDEX[1].pc4;
                    flushIF(IFID);
                    /**/fprintf(fptr, "Status: IF flushed.\n");
                    flushID(IDEX, &i1, &i2, &d1, &d2);
                    /**/fprintf(fptr, "Status: ID flushed.\n");
                }

                // * If branch misprediction occured, invalidate instructions in ID and IF
                if(br_cond && !IDEX[1].fetch_src && (IDEX[1].isBeq || IDEX[1].isBne || IDEX[1].isJal || IDEX[1].isJr || IDEX[1].isJump))
                {
                    // * Pass the mispredicted pc back to IF
                    pc = IDEX[1].pc;

                    // TODO: Output Result & Stats (Jump & Branch)
                    /**/fprintf(fptr, "Status: Branch mispredicted. Jumping back to 0x%x\n", pc);
                    if(cycle_count == 3) goto endCycle;

                    // * Invalidate the instructions in ID and IF
                    IDEX[0].valid = false;
                    IFID[0].valid = false;

                    // ? TECHNICALLY, we do not need validation then? because invalidate = flush? consider this.
                    flushIF(IFID);
                    /**/fprintf(fptr, "Status: IF flushed.\n");
                    flushID(IDEX, &i1, &i2, &d1, &d2);
                    /**/fprintf(fptr, "Status: ID flushed.\n");
                }
            }

            /**/fprintf(fptr, "\nStatus: Passing latches...\n");
            pass_latches(IFID, IDEX, EXMEM, MEMWB);
            /**/fprintf(fptr, "Status: Latches passed.\n\n\n\n");

            check_stall(IFID, IDEX, EXMEM, MEMWB, reg, isShift, i1, i2);

            // * Check the necessity for data forwarding 
            if(MEMWB[1].write_reg == i1 && reg[i1].valid == false) reg[i1].valid = true;
            if(MEMWB[1].write_reg == i2 && reg[i2].valid == false) reg[i2].valid = true;
            if(!reg[i1].valid) fprintf(fptr, "***Register [%d] has been made invalid!***\n\n\n\n", i1);
            if(!reg[i2].valid) fprintf(fptr, "***Register [%d] has been made invalid!***\n\n\n\n", i2);

            // * Reset stall and check any invalid register (set stall to true if found)
            stall = false;
            for(int i = 0; i < REG_COUNT; i++)
            {
                if(!reg[i].valid)
                {
                    stall = true;
                    break;
                }
            }

            if(pc == 0xffffffff)
            {
                // TODO: Invalidate all instructions (pc, control signals, registers) after this one
                if(excess_cycles == 4) halt_IF = true;
                if(excess_cycles == 3) halt_ID = true;
                if(excess_cycles == 2) halt_EX = true;
                if(excess_cycles == 1) halt_MEM = true;
                if(excess_cycles == 0) break;
                excess_cycles -= 1;
            }
    }
    fclose(fptr);
    return 0;
}