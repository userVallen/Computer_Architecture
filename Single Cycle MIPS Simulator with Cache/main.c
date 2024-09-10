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
#define WAY_COUNT 4
#define SET_COUNT 16

// * Initialize a 16 MB memory, a program counter, 32 registers with 0 values, and an instruction
int mem[0x400000];
int pc = 0;
int reg[32] = {0, };
struct _instruction inst;
struct _cache cache[WAY_COUNT][SET_COUNT];

unsigned int inst_count, r_count, i_count, j_count, mem_count, br_count = 0;

// * control signals
int reg_dst, mem_read, mem_to_reg, mem_write, ALU_src, reg_write = 0;
char ALU_op;
bool isJump, isBranch, isJr, isJal, isShift, isSet, isLui, isUnsigned = false;

// * values
int write_reg, write_data, ALU_in, target, ALU_out, mem_out = 0;
bool br_cond, set_cond = false;

// * cache-related variables
bool cache_hit = false;
int out_data = 0;
unsigned int tag, hit_count, miss_count = 0;
unsigned char idx, offset, line_index, plane_index, oldest_index = 0;
struct _cache *victim_cache_line = NULL;
struct _cache *oldest_cache_line[SET_COUNT];

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
        if(i > MEM_SIZE)
        {
            fprintf(stderr, "EXCEPTION OCCURRED: Memory out of bounds\n");
            exit(EXIT_FAILURE);
        }

        int ret = fread(&var, sizeof(var), 1, fp);
        if(ret == 0) break;

        // ! handle exception for invalid instruction
        if(sizeof(ret) != sizeof(int))
        {
            fprintf(stderr, "EXCEPTION OCCURRED: Invalid instruction\n");
            exit(EXIT_FAILURE);
        }
        // printf("fread binary: 0x%x\n", var);

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

    // * Initialize SP(Reg[29]) to stack's starting point, RA (Reg[31]) to -1 (end of execution), and the rest to 0
    initialize_registers();

    while(pc != 0xffffffff)
    {
        /**/ if(inst_count > 80) break;
        reset_state();

        /**/ printf("PC: 0x%x\n", pc);

        // TODO: Instruction Fetch
        // ! handling exception for out of bounds memory access
        if((pc / 4) < 0 || ((pc / 4) > MEM_SIZE))
        {
            fprintf(stderr, "EXCEPTION OCCURRED: Memory access out of bounds\n");
            exit(EXIT_FAILURE);
        }
        inst.instruction = mem[pc/4];

        // TODO: Instruction Decode
        decode_instruction();

        // ! handling exception for out of bounds register access
        if(inst.rs < 0 || inst.rt < 0 || inst.rd < 0 || inst.rs > 31 || inst.rt > 31 || inst.rd > 31)
        {
            fprintf(stderr, "EXCEPTION OCCURRED: Register access out of bounds\n");
            exit(EXIT_FAILURE);
        }

        classify_instruction();

        // TODO: Execute/ALU op
        assign_control_signals();
        execute_instruction();
        set_target();

        // ! handling exceptions for out of bounds memory access
        if((target > MEM_SIZE) & (target != 0xffffffff))
        {
            fprintf(stderr, "EXCEPTION OCCURRED: Memory access out of bounds\n");
            exit(EXIT_FAILURE);
        }
        pc = target;

        // TODO: Memory Access (Load/Store)
        // ! Exception for out of bounds memory access (SW and LW)
        if(mem_write == 1 || mem_to_reg == 1)
        {
            if((ALU_out / 4) < 0 || ((ALU_out / 4) > MEM_SIZE))
            {
                fprintf(stderr, "EXCEPTION OCCURRED: Memory access out of bounds\n");
                exit(EXIT_FAILURE);
            }
        }

        // * divide the address (from ALU_out) into 3 parts and check if the accessed address is available in cache
        if(mem_read || mem_to_reg || mem_write)
        {
            divide_address();
            check_cache();
            /**/printf("%s\n", (cache_hit) ? "cache hit" : "cache miss");
        }

        // * read from memory/cache
        if(mem_read || mem_to_reg)
        {
            if(cache_hit) // * cache hit
            {
                hit_count++;
                out_data = read_cache(); // * read data from cache
                printf("data from cache[%d][%d].data[%d] is %d\n", plane_index, line_index, offset/4, out_data);
            }
            else // * cache miss
            {
                miss_count++;
                find_victim(); // * look for victim cache line
                write_cache(victim_cache_line); // * fill in/replace cache
                out_data = read_cache(); // * read the updated cache
            }
        }

        // * write in memory/cache
        if(mem_write)
        {
            if(cache_hit) // * cache hit
            {
                hit_count++;
                write_cache(&cache[plane_index][line_index]);
            }
            else // * cache miss
            {
                miss_count++;
                find_victim(); // * look for victim cache line
                write_cache(victim_cache_line); // * fill in/replace cache
            }
        }

        set_output();

        // TODO: Write Back Result to Registers
        if(reg_write) reg[write_reg] = write_data;

        // TODO: Output Result & Stats
        if(mem_write) printf("CHANGE OCCURRED: Memory[%d] = %d\n", (ALU_out/4), mem[ALU_out/4]);
        if(reg_write) printf("CHANGE OCCURRED: Register[%d] = %d\n", write_reg, reg[write_reg]);
        if(isJump || isJal || isJr || (isBranch & br_cond)) printf("Jumped to 0x%x\n", target);
        showStats();

        // ? Debug (log file)
        #ifdef DEBUG_PC
        if(inst.opcode == 0x2 || inst.opcode == 0x3) fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nADDRESS: 0x%x\nPC: 0x%x\n\n", inst_count, inst.opcode, (inst.instruction & 0x3ffffff), pc);
        else if(!inst.opcode) fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nFUNC = 0x%x\nRS: %d\nRT: %d\nRD: %d\nSHAMT: %d\nPC: 0x%x\n\n",inst_count, inst.opcode, inst.func, inst.rs, inst.rt, inst.rd, inst.shamt, pc);
        else fprintf(fptr, "Instruction: %d\nOPCODE: 0x%x\nRS: %d\nRT: %d\nIMM: 0x%x\nPC: 0x%x\n",inst_count, inst.opcode, inst.rs, inst.rt, inst.imm, pc);

        fprintf(fptr, "Displaying all registers:\n");
        for(int i = 0; i < 8; i++)
        {
            for(int j = 0; j <= 24 ; j+=8)
            {
                fprintf(fptr, "reg[%02d]: %-8d 0x%-16x", (i + j), reg[i + j], reg[i + j]);
            }
            fprintf(fptr, "\n");
        }
        fprintf(fptr, "\n");
        #endif
    }

    printf("Hit count: %d\nMiss count: %d\n", hit_count, miss_count);

    // * Reset all values to initial state
    initialize_registers();
    initialize_cache();
    reset_state();

    fclose(fptr);
    return 0;
}