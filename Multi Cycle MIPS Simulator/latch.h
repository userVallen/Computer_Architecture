// #ifndef LATCH_H
// #define LATCH_H

// #include <stdbool.h>
// #include "instruction.h"

// // struct _ifid
// // {
// //     int pc4;
// //     struct _instruction inst;
// // };

// // struct _idex
// // {
// //     int address;
// //     int pc4;
// //     int v1;
// //     int v2;
// //     int s_imm;
// //     int writeReg;
// //     int ext_imm;
    
// //     // ? Control Signals
// //     bool isJr;
// //     bool isJump;
// //     bool isBeq;
// //     bool isBne;
// //     bool isJal;
// //     bool isSet;
// //     bool isLui;
// //     int RegWrite;
// //     int MemWrite;
// //     int MemRead;
// //     int MemtoReg;
// //     int ALUSrc;
// //     char ALUOp;

// //     int rs;
// //     int rt;
// //     // int instIndex;

// // };

// // struct _exmem
// // {
// //     int pc4;
// //     int v2;
// //     int writeReg;
// //     int ext_imm;
// //     int setCond;
// //     int ALUOut;

// //     bool isJal;
// //     bool isSet;
// //     bool isLui;
// //     int RegWrite;
// //     int MemWrite;
// //     int MemRead;
// //     int MemtoReg;
// // };

// // struct _memwb
// // {
// //     int pc4;
// //     int v2;
// //     int writeReg;
// //     int ext_imm;
// //     int setCond;
// //     int ALUOut;
// //     int memOut;

// //     bool isJal;
// //     bool isSet;
// //     bool isLui;
// //     int RegWrite;
// //     int MemWrite;
// //     int MemtoReg;
// // };

// /*
// write in main:

// fetch
// decode(inputLatch, outputLatch) --> inputLatch would be something like currentLatch[1], outputLatch nextLatch[0]
// execute
// memaccess
// writeback

// flushLatches();
// cycle++;

// */

// #endif