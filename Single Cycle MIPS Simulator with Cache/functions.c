#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "variables.h"
#define REG_COUNT 32
#define SET_COUNT 16
#define WAY_COUNT 4
#define CACHE_LINE_SIZE 16
#define MEM_SIZE 0x400000

void showStats()
{
    printf("Displaying all registers:\n");
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j <= 24 ; j+=8)
        {
            printf("reg[%02d]: %-8d 0x%-16x", (i + j), reg[i + j], reg[i + j]);
        }
        printf("\n");
    }
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nNumber of instructions executed: %d\nNumber of R-type instructions executed: %d\nNumber of I-type instructions executed: %d\nNumber of J-type instructions executed: %d\nNumber of memory access instructions executed: %d\nNumber of branches taken: %d\n\n", inst_count, r_count, i_count, j_count, mem_count, br_count);
}

int select_MUX(int control, int arg_count, ...)
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

void reset_state()
{
    reg_dst = 0;
    mem_read = 0;
    mem_to_reg = 0;
    mem_write = 0;
    ALU_src = 0;
    reg_write = 0;

    isJump = false;
    isBranch = false;
    isJr = false;
    isJal = false;
    isShift = false;
    isSet = false;
    isLui = false;
    isUnsigned = false;

    victim_cache_line = NULL;
}

void classify_instruction()
{
    inst_count += 1;
    if(inst.opcode == 0x2 || inst.opcode == 0x3 || ((inst.opcode == 0x0) && (inst.func == 0x08)) || ((inst.opcode == 0x0) && (inst.func == 0x09))) j_count += 1;
    if(inst.opcode == 0x4 || inst.opcode == 0x5 || inst.opcode == 0x8 || inst.opcode == 0x9 || inst.opcode == 0xa || inst.opcode == 0xb || inst.opcode == 0xc || inst.opcode == 0xd || inst.opcode == 0xf || inst.opcode == 0x23 || inst.opcode == 0x2b) i_count += 1;
    if(inst.opcode == 0x0) r_count += 1;
    if(inst.opcode == 0x23 || inst.opcode == 0x2b) mem_count += 1;
}

void decode_instruction()
{
    inst.opcode = (inst.instruction >> 26) & 0x3f;
    inst.rs = (inst.instruction >> 21) & 0x1f;
    inst.rt = (inst.instruction >> 16) & 0x1f;
    inst.rd = (inst.instruction >> 11) & 0x1f;
    inst.shamt = (inst.instruction >> 6) & 0x1f;
    inst.func = inst.instruction & 0x3f;
    inst.imm = inst.instruction & 0xffff;
    inst.s_imm = (inst.imm & 0x8000) ? (inst.imm | 0xffff0000) : (inst.imm);
    if(inst.opcode == 0xc || inst. opcode == 0xd) inst.ext_imm = inst.imm | 0x00000000;
    if(inst.opcode == 0xf) inst.ext_imm = inst.imm << 16;
}

void execute_instruction()
{
    // * set destination register
    write_reg = select_MUX(reg_dst, 3, inst.rt, inst.rd , 31);

    // * set second ALU input
    ALU_in = select_MUX(ALU_src, 3, reg[inst.rt], inst.s_imm, inst.ext_imm);

    // * calculate set condition
    if(inst.opcode == 0xa || inst.opcode == 0xb || (inst.opcode == 0 && (inst.func == 0x2a || inst.func == 0x2b))) set_cond =  (reg[inst.rs] < ALU_in) ? 1 : 0;
    
    // * calculate ALU output
    if(isShift) ALU_out = set_ALU_op(ALU_op, reg[inst.rt], inst.shamt);
    else ALU_out = set_ALU_op(ALU_op, reg[inst.rs], ALU_in);
}

void set_output()
{
    // * set memory output
    // if(mem_to_reg) mem_out = mem[out_data/4];

    // * set final output
    write_data = select_MUX(mem_to_reg, 2, ALU_out, out_data);
    write_data = select_MUX(isJal, 2, write_data, (pc + 8));
    write_data = select_MUX(isSet, 2, write_data, set_cond);
    write_data = select_MUX(isLui, 2, write_data, inst.ext_imm);
}

void set_target()
{
    if(inst.opcode == 0x4)
    {
        if (reg[inst.rs] == reg[inst.rt])
        {
            br_cond = true;
            br_count += 1;
        }
        else br_cond = false;
    }
    if(inst.opcode == 0x5)
    {
        if (reg[inst.rs] != reg[inst.rt])
        {
            br_cond = true;
            br_count += 1;
        }
        else br_cond = false;
    }

    target = select_MUX((isBranch & br_cond), 2, (pc + 4), ((pc + 4) + (inst.s_imm << 2)));
    target = select_MUX(isJump, 2, target, ((((pc + 4) & 0xf0000000) >> 28) | ((inst.instruction & 0x3ffffff) << 2)));
    target = select_MUX(isJr, 2, target, reg[inst.rs]);
}

void assign_control_signals()
{
        // * isJr
    isJr = (inst.opcode == 0x0 && (inst.func == 0x08 || inst.func == 0x09)) ? true : false;
        // * isJump
    isJump = (inst.opcode == 0x2 || inst.opcode == 0x3) ? true : false;
        // * isBranch
    isBranch = (inst.opcode == 0x4 || inst.opcode == 0x5) ? true : false;
        // * isJal
    isJal = (inst.opcode == 0x3 || ((inst.opcode == 0x0) && (inst.func == 0x09))) ? true : false;
        // * isSet
    isSet = (inst.opcode == 0xa || inst.opcode == 0xb || ((inst.opcode == 0x0) && ((inst.func == 0x2a) || (inst.func == 0x2b)))) ? true : false;
        // * isLui
    isLui = (inst.opcode == 0xf) ? true : false;
        // * isShift
    isShift = (inst.opcode == 0x0 && ((inst.func == 0x00) || (inst.func == 0x02))) ? true : false;
        // * reg_dst
    if(inst.opcode == 0x0 && !isJr) reg_dst = 1;
    else if(inst.opcode == 0x3 || ((inst.opcode == 0x0) && (inst.func == 0x09))) reg_dst = 2;
    else reg_dst = 0;
        // * reg_write
    reg_write = ((inst.opcode == 0x0 && inst.func == 0x08) || inst.opcode == 0x2 || inst.opcode == 0x4 || inst.opcode == 0x5 || inst.opcode == 0x2b) ? 0 : 1;
        // * mem_read
    mem_read = (inst.opcode == 0x23) ? 1 : 0;
        // * mem_write
    mem_write = (inst.opcode == 0x2b) ? 1 : 0;
        // * mem_to_reg
    mem_to_reg = (inst.opcode == 0x23) ? 1 : 0;
        // * ALU_src
    if(inst.opcode == 0x8 || inst.opcode == 0x9 || inst.opcode == 0xa || inst.opcode == 0xb || inst.opcode == 0x23 || inst.opcode == 0x2b) ALU_src = 1;
    else if(inst.opcode == 0xc || inst.opcode == 0xd) ALU_src = 2;
    else ALU_src = 0;
        // * ALU_op
    if(inst.opcode == 0x8 || inst.opcode == 0x9 || inst.opcode == 0x23 || inst.opcode == 0x2b || ((inst.opcode == 0x0) && ((inst.func == 0x20) || (inst.func == 0x21)))) ALU_op = '+';
    if(inst.opcode == 0x0 && ((inst.func == 0x22) || (inst.func == 0x23))) ALU_op = '-';
    if(inst.opcode == 0xc || ((inst.opcode == 0x0) && (inst.func == 0x24))) ALU_op = '&';
    if(inst.opcode == 0xd || ((inst.opcode == 0x0) && (inst.func == 0x25))) ALU_op = '|';
    if(inst.opcode == 0x0 && inst.func == 0x00) ALU_op = '<';
    if(inst.opcode == 0x0 && inst.func == 0x02) ALU_op = '>';
    if(inst.opcode == 0x0 && inst.func == 0x27) ALU_op = '~';
}

void initialize_registers()
{
    for(int i = 0; i < 32; i++)
    {
        switch(i)
        {
            case 29:
                reg[i] = 0x1000000;
                break;
            case 31:
                reg[i] = 0xffffffff;
                break;
            default:
                reg[i] = 0;
                break;
        }
    }
}

void initialize_cache()
{
    for(int i = 0; i < WAY_COUNT; i++)
    {
        for(int j = 0; j < SET_COUNT; j++)
        {
            cache[i][j].valid = false;
            cache[i][j].dirty = false;
            cache[i][j].sca = false;
            cache[i][j].tag = 0;
            cache[i][j].order_of_entrance = 0;
            cache[i][j].starting_address = 0;
            for(int k = 0; k < 4; k++) cache[i][j].data[k] = 0;
        }
    }

    for(int i = 0; i < SET_COUNT; i++)
    {
        oldest_cache_line[i] = NULL;
    }
}

int read_mem(int address)
{
    return mem[address/4];
}

void write_mem(int address, int value)
{
    mem[address/4] = value;
}

void divide_address()
{
    // * divide the address into tag, idx, and offset
    tag = ALU_out >> 8;
    idx = (ALU_out >> 4) & 0xf;
    offset = ALU_out & 0xf;
    /**/ printf("ALU_out: 0x%x (%d [%d])\ntag: 0x%x (%d)\nindex: 0x%x (%d)\noffset: 0x%x (%d)\n\n", ALU_out, ALU_out, ALU_out/4, tag, tag, idx, idx, offset, offset);
}

void check_cache()
{
    line_index = idx;
    for(int i = 0; i < WAY_COUNT; i++)
    {
        if(cache[i][line_index].tag == tag) // * in the case of cache hit
        {
            cache_hit = true;
            plane_index = i;
            cache[i][line_index].sca = true;
            break;
        }
        else if (i == (WAY_COUNT - 1)) cache_hit = false; // * in the case of cache miss
    }
}

int read_cache()
{
    /**/printf("offset is %d\n", offset);
    return cache[plane_index][line_index].data[offset / 0x4];
}

void write_cache(struct _cache *cache_line)
{
    // * fetch 16B of data from memory starting from the starting address
    cache_line->starting_address = ALU_out & 0xffffffe0; // * set the starting address
    for(int i = 0; i < (CACHE_LINE_SIZE / 4); i++)
    {
        if(((cache_line->starting_address + 0x8 * i) / 4) > MEM_SIZE)
        {
            fprintf(stderr, "EXCEPTION OCCURED: Memory access out of bounds\n");
            exit(EXIT_FAILURE);
        }

        // * copy the whole cache line from memory except for the offset being updated (fetch from rt)
        if(i == (offset / 0x4))
        {
            memcpy(&cache_line->data[i], &reg[inst.rt], 4);
            printf("new data is %d --> ", reg[inst.rt]);
        }
        else memcpy(&cache_line->data[i], &mem[(cache_line->starting_address + 0x8 * i) / 4], 4);
        /**/ printf("cache[%d][%d].data[%d] is %d from mem[%d]\n", plane_index, line_index, i, cache_line->data[i], (cache_line->starting_address + 0x8 * i) / 4);
    }
    cache_line->tag = ALU_out >> 8; // * set the tag bit
    cache_line->dirty = true; // * mark the cache line as dirty
    cache_line->sca = false; // * reset the SCA bit
    printf("\n");
}

void update_order()
{
    // * decrement the order of each cache line in the set except for the oldest cache line (reset as the youngest at the cost of SCA bit)
    for(int i = 0; i < WAY_COUNT; i++)
    {
        if(cache[i][line_index].order_of_entrance == 1) cache[i][line_index].order_of_entrance = 4;
        else cache[i][line_index].order_of_entrance -= 1;
    }
}

unsigned char find_oldest()
{
    int oldest = 0;

    // * look for the oldest cache line within the given set
    for(oldest = 0; oldest < WAY_COUNT; oldest++)
    {
        if(cache[oldest][line_index].order_of_entrance == 1)
        {
            // * mark this cache line as the oldest cache line of this set
            oldest_cache_line[oldest] = &cache[oldest][line_index];
            if(oldest_cache_line[oldest]->sca) // * check if the oldest cache line has SCA bit
            {
                // * mark the oldest cache line's SCA as false
                oldest_cache_line[oldest]->sca = false;

                // * update the order, restart the search, and look for the oldest cache line
                update_order();
                oldest = 0;
            }
            else break; // * save the index of the oldest cache line and break the loop
        }
    }

    return oldest;
}

void find_victim()
{
    bool found = false;
    unsigned char order = 1;

    // * count how many valid cache lines are currently available (to determine the youngest recency, i.e. which way to write in)
    for(int i = 0; i < WAY_COUNT; i++)
    {
        if(cache[i][line_index].valid) order++;
    }

    if(order <= WAY_COUNT)
    {
        // ? for cold miss
        for(int i = 0; i < WAY_COUNT; i++)
        {
            // * find the first instance of an invalid cache line (if there is any)
            if(!cache[i][line_index].valid) // * select the invalid cache line as the victim and mark it as valid
            {
                victim_cache_line = &cache[i][line_index];
                victim_cache_line->valid = true;
                found = true;
                victim_cache_line->order_of_entrance = order; // * mark the recency of this cache line in respect to the other cache lines in the corresponding set
                /**/ printf("order = %d, \n", order);
                break;
            }
        }
    }
    else
    {
        // ? for conflict miss
        // * in the case of no invalid cache line found within the given set
        if(!found)
        {
            oldest_index = find_oldest(); // * look for the oldest cache line within the given set
            
            // * mark the oldest cache line as the victim
            victim_cache_line = oldest_cache_line[oldest_index];

            // * check if the victim cache line is dirty
            if(victim_cache_line->dirty)
            {
                // * copy 16B of data from the cache line to the memory starting from the corresponding starting address
                victim_cache_line->starting_address = ALU_out & 0xffffffe0;
                for(int i = 0; i < (CACHE_LINE_SIZE / 4); i++)
                {
                    memcpy(&mem[cache[oldest_index][line_index].starting_address + 0x8 * i], &victim_cache_line->data[i], 4);
                }
                victim_cache_line->dirty = false; // * mark the victim cache line as not dirty
            }
        }
    }
}

// void check_dirty(struct _cache *cache_line)
// {
//     // * check if the victim cache line is dirty
//     if(cache_line->dirty)
//     {
//         // * copy 16B of data from the cache line to the memory starting from the corresponding starting address
//         cache_line->starting_address = ALU_out & 0xffffffe0;
//         for(int i = 0; i < (CACHE_LINE_SIZE / 4); i++)
//         {
//             // memcpy(&mem[cache[oldest_index][line_index].starting_address + 0x8 * i], &cache_line->data[i], 4);
//             mem[cache[oldest_index][line_index].starting_address + 0x8 * i] = cache_line->data[i];
//         }
//         cache_line->dirty = false; // * mark the victim cache line as not dirty
//     }
// }