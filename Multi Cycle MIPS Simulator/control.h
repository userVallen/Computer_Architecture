#ifndef CONTROL_H
#define CONTROL_H

struct _control
{
    bool isJr;
    bool isJump;
    bool isBeq;
    bool isBne;
    bool isJal;
    bool isSet;
    bool isShift;
    int RegDst;
    int RegWrite;
    int MemRead;
    int MemWrite;
    int MemtoReg;
    int ALUSrc;
    char ALUOp;

};

#endif