#include <stdio.h>
#include "data_structures.h"

typedef struct opcode opcode;

void first_transition(FILE* file, char *argsName);

void searchLine(char buff[], FILE* file);

int checkLabel(char buff[]);

char* checkData(char buff[], char **instruction);

/* char* checkInstruction(char buff[]); */

int validateLabel(char buff[], int type);

char* searchOperand(int type);

int validateDataType(char type[]);

char* searchString(char buff[]);

char *findRegister(char buff[]);

char *findOperandType(char buff[]);

char *findOpCode(char command[]);

int getInstructionType(char buff[]);

int validateEnding();

char* findOperand(int *result, char buff[],int type);

int findUserError(int operandType1, int operandType2, int numberOfCommas);

int findParameters(char **param1, char **param2);

int verifyParameters(char buff[]);

int writeMachineCode(int instructionNumber,char* instructionCode, char* operand1,char* operand2, int operandType1, int operandType2, char *jmpLabel,FILE *file);

int validateNumber(char buff[],int type);

void freeAllLists();

void freeList(node *head);

void printError(int linesRead, char* fileName);

int validateEntryExtern(char labelName[], int type);

char *searchData();

int checkEmptyLine(char buff[]);

int findNumberOfCommas(char buff[]);

int storeData(int flag,int numberOfCommas, node *newNode);

void addData(FILE *file);

int verifyEntryUse();

void writeOperands(int operandType1, int operandType2, char *operand1, char *operand2, FILE *file);