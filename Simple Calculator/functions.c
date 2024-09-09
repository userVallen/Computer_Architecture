#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "variables.h"

// * display all register values
void displayReg()
{
    for (int i = 0; i < 10; i++) { printf("R[%d]: %d (0x%X)\n", i, reg[i], reg[i]); }
    printf("\n");
}

// * check if an operand is an immediate
int isImm(int i, char *_c[])
{
    if (_c[i][0] == '0' && _c[i][1] == 'x') return 1;
    else return 0;
}

// * check if an operand is a register
int isReg(int i, char *_c[])
{
    if (_c[i][0] == 'R' && isdigit(_c[i][1])) return 1;
    else return 0;
}

// * read an immediate value
int readImm(char *_c)
{
    char *endptr;
    long num;

    num = strtol(_c, &endptr, 16);

    return num;
}

// * read a register value
int readReg(int index, char *_c[])
{
    int num;
    num = _c[index][1] - '0';
    num = reg[num];

    return num;
}

// * check a register and copies its value
void checkReg(int index, char *_c[], int *_op)
{
    // * check if an operand is a register
    if(isReg(index, _c)) { *_op = readReg(index, _c); }
    // ! handle exception for invalid register format
    else
    {
        perror("EXCEPTION OCCURED: Incorrect register format --> R<reg_number>\n");
        exit(EXIT_FAILURE);
    }
}

// * check an operand and copies its value
void checkOp(int index, char *_c[], int *_op)
{
    // * check if an operand is an immediate or a register
    if (isImm(index, _c)) { *_op = readImm(_c[index]); }
    else if (isReg(index, _c)) { *_op = readReg(index, _c); }
    // ! handle the exception for invalid input format
    else
    {
        perror("EXCEPTION OCCURED: Incorrect input format --> 0x<imm_number> or R<reg_number>\n");
        exit(EXIT_FAILURE);
    }
}

int gcd(int _op1, int _op2)
{
    if(_op2 == 0) { return _op1; }
    if(_op1 == 0) { return _op2; }
    return gcd(_op2, (_op1 % _op2));
}