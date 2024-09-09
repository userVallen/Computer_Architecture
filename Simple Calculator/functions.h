#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void displayReg();
int isImm(int i, char *_c[]);
int isReg(int i, char *_c[]);
int readImm(char *_c);
int readReg(int index, char *_c[]);
void checkReg(int index, char *_c[], int *_op);
void checkOp(int index, char *_c[], int *_op);
int gcd(int _op1, int _op2);

#endif