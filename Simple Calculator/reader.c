#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "variables.h"
#include "functions.h"
#define REG_SIZE 100
#define TEMP_SIZE 4

// * create stack (inst_reg) to store instructions
int top = -1;
char *inst_reg[REG_SIZE];

// * create registers and initialize all values to 0
int reg[10] = {0};

int main(int argc, char *argv[])
{
    // ! handle exception for invalid usage format
    if (argc != 2)
    {
        fprintf(stderr, "EXCEPTION OCCURED: Incorrect usage --> %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // * reading files & executing
    FILE *fp;
    fp = fopen(argv[1], "r");
    // ! handle exception for missing file
    if (fp == NULL)
    {
        fprintf(stderr, "EXCEPTION OCCURED: File '%s' not found\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // * create an instruction pointer
    char **inst_ptr = &inst_reg[0];

    // * allocate memory for inst_reg
    for (int i = 1; i < REG_SIZE; i++) { inst_reg[i] = malloc(sizeof(char) * 3 + sizeof(int) * 5); }

    // * create an array to hold instruction tokens (max. 1 opcode + 3 arguments)
    char *temp[TEMP_SIZE];
    for (int i = 0; i < TEMP_SIZE; i++) { temp[i] = malloc(sizeof(int) * 5); }

    // * for getline()
    size_t buffer = 0;
    ssize_t indicator;

    // * for strtok()
    char *token = NULL;

    printf("-----------------------\n\tREADER\n-----------------------\n\n");

    while (1)
    {
        // * read next instruction and stack to inst_reg
        top++;
        if (top >= REG_SIZE)
        {
            perror("EXCEPTION OCCURED: Too many instructions --> Max. number of instructions is 100\n");
            exit(EXIT_FAILURE);
        }
        indicator = getline(&inst_reg[top], &buffer, fp);
        // ? if there is no more line to read in the file
        if(indicator == -1)
        {
            top--;
            strcat(inst_reg[top], "\n");
            break;
        }
    }
    fclose(fp);

    // * print out the contents of inst_reg
    printf("CONTENTS OF INST_REG:\n");
    for (int i = 0; i <= top; i++) { printf("%s", inst_reg[i]); }
    printf("\n\n");

    // * define "inst" as a copy instruction to be processed
    char *inst = malloc(sizeof(char) * 3 + sizeof(int) * 5);

    int *op1 = malloc(sizeof(int) * 5);
    int *op2 = malloc(sizeof(int) * 5);
    
    // * process instructions from inst_reg
    for (int i = 0; i <= top; i++)
    {
        inst_ptr = &inst_reg[i];

        // * print the current instruction
        printf("-•••-\n\nCurrent instruction (line %d): %s\n", (i + 1), *inst_ptr);

        // * create a copy of the instruction to be processed
        strcpy(inst, inst_reg[i]);

        // * tokenize the instruction and store the tokens in temp[]
        token = strtok(inst, " ");
        for (int i = 0; token != NULL; i++)
        {
            strcpy(temp[i], token);
            token = strtok(NULL, " ");
        }

        // * check operation (opcode)
        if (strcmp(temp[0], "+") == 0)
        {
            checkOp(1, temp, op1);
            checkOp(2, temp, op2);

            // * move the result to register R0 and print it
            reg[0] = (*op1 + *op2);
            printf("R0: %d = %d + %d\n\n", reg[0], *op1, *op2);

            displayReg();
        }
        else if (strcmp(temp[0], "-") == 0)
        {
            checkOp(1, temp, op1);
            checkOp(2, temp, op2);

            // * move the result to register R0 and print it
            reg[0] = (*op1 - *op2);
            printf("R0: %d = %d - %d\n\n", reg[0], *op1, *op2);

            displayReg();
        }
        else if (strcmp(temp[0], "*") == 0)
        {
            checkOp(1, temp, op1);
            checkOp(2, temp, op2);

            // * move the result to register R0 and print it
            reg[0] = (*op1 * *op2);
            printf("R0: %d = %d * %d\n\n", reg[0], *op1, *op2);

            displayReg();
        }
        else if (strcmp(temp[0], "/") == 0)
        {
            checkOp(1, temp, op1);
            checkOp(2, temp, op2);
            // ! handle exception for the case of division by zero
            if(op2 == 0) 
            {
                perror("EXCEPTION OCCURED: Unable to divide by zero\n");
                exit(EXIT_FAILURE);
            }

            // * move the result to register R0 and print it
            reg[0] = (*op1 / *op2);
            printf("R0: %d = %d / %d\n\n", reg[0], *op1, *op2);

            displayReg();
        }
        else if (strcmp(temp[0], "M") == 0)
        {
            // * check if the first input is an immediate or a register
            checkOp(1, temp, op1);

            // * check register format
            if (isReg(2, temp))
            {
                *op2 = temp[2][1] - '0';
                reg[*op2] = *op1;
            }
            // ! handle exception for invalid register format
            else
            {
                perror("EXCEPTION OCCURED: Incorrect register format --> R<reg_number>\n");
                exit(EXIT_FAILURE);
            }

            printf("R%d: %d\n\n", *op2, *op1);

            displayReg();
        }
        else if (strcmp(temp[0], "C") == 0)
        {
            checkOp(1, temp, op1);
            checkOp(2, temp, op2);

            if(*op1 == *op2) 
            { 
                reg[0] = 0; 
                printf("%d = %d\n\n", *op1, *op2);
            }
            else if (*op1 > *op2 ) 
            { 
                reg[0] = 1; 
                printf("%d > %d\n\n", *op1, *op2);
            }
            else 
            {
                reg[0] = -1;
                printf("%d < %d\n\n", *op1, *op2);
            }

            displayReg();
        }
        else if (strcmp(temp[0], "J") == 0)
        {
            // * check if the first input is an integer (line number)
            // ? "-1" to compensate for stack starting from 0
            if (atoi(temp[1] - 1) >= 0 && atoi(temp[1] - 1) <= top)
            {
                // ? "- 2" to compensate for stack starting from 0 and increment from for in the next iteration
                i = atoi(temp[1]) - 2;
                printf("Jumping to line %d\n\n", atoi(temp[1]));
            }
            // ! handle exception for invalid target line
            else
            {
                perror("EXCEPTION OCCURED: Target line not found\n");
                exit(EXIT_FAILURE);
            }
        }
        else if(strcmp(temp[0], "BEQ") == 0)
        {
            if(reg[0] == 0)
            {
                // * check if the first input is an integer (line number)
                // ? "-1" to compensate for stack starting from 0
                if (atoi(temp[1] - 1) >= 0 && atoi(temp[1] - 1) <= top)
                {
                    // ? "- 2" to compensate for stack starting from 0 and increment from for in the next iteration
                    i = atoi(temp[1]) - 2;
                    printf("R[0] = 0, jumping to line %d\n\n", atoi(temp[1]));
                }
                // ! handle exception for invalid target line
                else
                {
                    perror("EXCEPTION OCCURED: Target line not found");
                    exit(EXIT_FAILURE);
                }
            }
            else { printf("R[0] ≠ 0, ignoring branch and proceeding with the next instruction\n\n"); }
        }
        else if(strcmp(temp[0], "BNE") == 0)
        {
            if(reg[0] != 0)
            {
                // * check if the first input is an integer (line number)
                // ? "-1" to compensate for stack starting from 0
                if (atoi(temp[1] - 1) >= 0 && atoi(temp[1] - 1) <= top)
                {
                    // ? "- 2" to compensate for stack starting from 0 and increment from for in the next iteration
                    i = atoi(temp[1]) - 2;
                    printf("R[0] ≠ 0, jumping to line %d\n\n", atoi(temp[1]));
                }
                // ! handle exception for invalid target line
                else
                {
                    perror("EXCEPTION OCCURED: Target line not found");
                    exit(EXIT_FAILURE);
                }
            }
            else { printf("R[0] = 0, ignoring branch and proceeding with the next instruction\n\n"); }
        }
        else if(strcmp(temp[0], "GCD") == 0)
        {
            checkOp(1, temp, op1);
            checkOp(2, temp, op2);

            reg[0] = gcd(*op1, *op2);
            printf("GCD of %d and %d is %d\n\n", *op1, *op2, reg[0]);
        }
        else if(strcmp(temp[0], "H") == 0)
        {
            printf("Program halted. Terminating program.\n");
            exit(EXIT_SUCCESS);
        }
        // ! handle exception for invalid operation
        else
        {
            perror("EXCEPTION OCCURED: Incorrect operation --> +, -, *, /, M, C, J, BEQ, GCD, H\n");
            exit(EXIT_FAILURE);
        }
    }
    printf("\n-END OF FILE-\n");

    // * free allocated memories
    for (int i = 1; i < REG_SIZE; i++) { free(inst_reg[i]); }
    for (int i = 0; i < TEMP_SIZE; i++) { free(temp[i]); }
    free(inst);
    free(op1);
    free(op2);

    return 0;
}