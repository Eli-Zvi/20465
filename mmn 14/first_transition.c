#include "first_transition.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#define MAX_LENGTH 82 /* MAX LENGTH is 82 because of null termination character */
#define NUM_OF_OPCODES 16 /* NUMBER OF DIFFERENT OPERATION CODES */
#define NUM_OF_REGS 8 /* NUMBER OF DIFFERENT REGISTERS */
#define PC (IC+DC+100)

/* NOTE:
* memory/PC limit is commented out as the instructions in the project were unclear
* while writing to machine code file I could've used a number in the format to create the spaces but I did not because its easier to visualize
* white spaces and comment lines are removed by the pre_assembler as it was unclear if it mattered
* jsr/jmp/bne instructions are not allowed any spaces in parameters but the label we jump to is allowed a space after it
*/

struct node {
    char *name;
    int pc;
    int type; /* 0 - declared in file 0 - neither entry - 1 extern  - 2 */
    int numberOfCodes; /* for data node */
    char **code; /* used to save data machine code */
    struct node *next;
};

struct opcode{
    char *binary;
    char *name;
};

node *dataHead = NULL; /* will be used to save the machine code for the data part of the program - think about how to store label value because data is always at the end of the file*/
node *labelHead = NULL; /* will be used to save the labels of the program, maybe make another one for .entry and .extern ones */
node *entryExternHead = NULL;

int IC,DC,errCode;

const opcode oparr[]= {{"0000","mov"},{"0001", "cmp"}, {"0010", "add"}, {"0011", "sub"},{"0100","not"},
                       {"0101","clr"}, {"0110", "lea"}, {"0111", "inc"}, {"1000", "dec"}, {"1001", "jmp"},
                       {"1010", "bne"}, {"1011", "red"}, {"1100", "prn"}, {"1101", "jsr"},{"1110", "rts"},
                       {"1111", "stop"}};

const char* regarr[] = {"r0","r1","r2","r3","r4","r5","r6","r7"};

/*
 * main function of the file that gets a new line from the file and calls onto the searchLine function
 * @param fd - the macro file
 * @param argName - the name of the file
 */
void first_transition(FILE* fd, char *argName)
{
    FILE* file;
    char buff[MAX_LENGTH];
    char *mcFileName;
    int lastPC, argNameLength, linesRead, emptyFlag;
    
    IC = 0; DC = 0; errCode = 0; linesRead = 1, emptyFlag = 0;
    lastPC = PC;

    /* copy argName to mcFileName to create the name of the machine code file */
    argNameLength = strlen(argName);
    mcFileName = (char*) calloc((argNameLength + 4), sizeof(char));

    if(!mcFileName)
    {
        fprintf(stderr,"memory allocation error");
        exit(-1);
    }

    memcpy(mcFileName,argName,argNameLength);

    file = fopen(strcat(mcFileName,".mc"), "w"); /* machine code file */
    
    rewind(fd);

    while(!feof(fd))
    {
        if (fgets(buff, MAX_LENGTH, fd) != NULL) 
        { /*puts(buff);*/}
        else break;

        if(!checkEmptyLine(buff) && buff[0] != ';')
        {
            emptyFlag = 1; /* we've found a non comment/white space line so the file is not empty */

            /* can be commented back in if limiting the memory/PC is a need for the project but the instructions were unclear */
            /*if ((PC) > 256) {  can be commented out for testing
                errCode = -1;
                fprintf(stderr, "Error! all memory used too many instructions given!");
                break;
            } */

            if ((PC) != lastPC) { /* if we've added new code to machine code file we assign a next line char */
                fprintf(file, "\n");
                lastPC = (PC);
            }

            searchLine(buff, file);

            printError(linesRead, argName); /* prints error if one was found */
        }

        linesRead++;
    }

    if((PC) != lastPC)
    {
        fprintf(file,"\n");
    }

    if(!errCode) {
        addData(file); /* add the data segment of the code to the end of the file */
        verifyEntryUse(); /* verify that each .entry instruction's label was defined */
    }
    else verifyEntryUse();


    fclose(file);  /* close machine code file */
    fclose(fd);/* close am file as we've finished using it! */

    if(errCode == 0 && emptyFlag) {
        /* no errors found, now we make the .ob,.ext,.ent files */

        file = fopen(mcFileName,"r"); /* reopen machine code file! */
        secondTransition(dataHead, labelHead, entryExternHead, IC, DC, file, argName); /* start second transition */
        freeAllLists(); /* frees all lists */
        fclose(file); /* close machine code file for good */
        remove(mcFileName); /* machine code file is no longer necessary! */
        free(mcFileName); /* free the string containing its name */
        return;
    }
    else if(!emptyFlag)
    { /* if the file is empty display error and exit */
        fprintf(stderr,"Empty file given");
        remove(mcFileName);
        free(mcFileName);
        return;
    }
    else
    { /* error found and thus we remove the machine code file and exit */
        remove(mcFileName);
        free(mcFileName);
        freeAllLists();
        return;
    }
}

/*
 * obtains parameters for the instruction and sends them to decoding functions accordingly
 * @param buff - line read
 * @param file - machine code file to be written into
 */
void searchLine(char buff[], FILE *file)
{
    char *label = NULL, *temp = NULL, *instructionCode = NULL, *dataType,*operand1 = NULL,*operand2 = NULL,*jmpLabel = NULL;
    int flag, instructionNumber, operandType1, operandType2,numberOfCommas;
    node *newNode = NULL, *tempNode = NULL;

    operandType1 = -2; operandType2 = -2;
    
    numberOfCommas = findNumberOfCommas(buff);

    if(checkLabel(buff)) /* if a ':' was found in checkLabel, we've found a label*/
    { /* find the label's name and store it */
        temp = strtok(buff,":"); /* if we've found a label get its name */

        while(temp && isspace(*temp) &&!(strcmp(temp,"") == 0)) /* get rid of spaces */
        {
            temp++;
        }

        if(!validateEntryExtern(temp,0)) /* loop through entry and extern list to check for any clashing declarations */
            return;

        if(validateLabel(temp,0))
        {
            newNode = (node*)malloc(sizeof(node));

            if(!newNode)
            {
                /*error*/
                fprintf(stderr,"memory allocation error, program will exit");
                exit(-1);
            }
            else {
                label = (char*) malloc(sizeof(char)*(strlen(temp)+1));

                if(!label)
                {
                    /*error*/
                    fprintf(stderr,"memory allocation error, program will exit");
                    exit(-1);
                }

                strcpy(label,temp);
                newNode->name = label;
                newNode->pc = PC;
            }
        }
        else
        {
            /* error */
            errCode = 1;
            return;
        }
    }

    /* checks if its a data with a label or without*/
    if((label && (dataType = checkData(NULL, &instructionCode))) || (!label && (dataType = checkData(buff,&instructionCode))))
    {
        flag = validateDataType(dataType); /* flag = 0 -> error, flag = 1 -> data, flag = 2 -> string, flag = 3 -> entry, flag = 4 -> extern */

        if(storeData(flag,numberOfCommas,newNode) == -1)
        {
            if(newNode)
            {
                if(newNode->name)
                    free(newNode->name);
                free(newNode);
                newNode = NULL;
            }
            return;
        }
    }
    else
    {
        instructionNumber = getInstructionType(instructionCode);

        if (instructionNumber > 13)
        {
            if(!validateEnding())
            {
                /* error */
                errCode = 5;
                goto error;
            }
            temp = convertToBinary(instructionNumber,4,0);

            fprintf(file,"0%d    0000%s000000",PC, temp);

            free(temp);
            temp = NULL;
            IC++;

        } else if (instructionNumber == -1) {
            /* error */
            errCode = 7;
            goto error;
        }
        else
        {
            if(instructionNumber != 9 && instructionNumber != 10 && instructionNumber != 13) {
                operand1 = findOperand(&operandType1, NULL, 1);

                if(operandType1 == 0 && !operand1)
                {
                    /* error */
                    goto error;
                }

                if (instructionNumber < 4 || instructionNumber == 6) {
                    operand2 = findOperand(&operandType2, NULL,1);
                    if(operandType2 == 0 && !operand2)
                    {
                        /* error */
                        goto error;
                    }
                }

                if (findUserError(operandType1, operandType2,numberOfCommas)) {
                    /* error */
                    goto error;
                }

                if(writeMachineCode(instructionNumber,instructionCode,operand1,operand2,operandType1,operandType2,NULL,file) == -1)
                {
                    /* error */
                    goto error;
                }
            }
            else
            {
                jmpLabel = strtok(NULL,"'('\n");
                if(findParameters(&operand1,&operand2) == -1)
                {
                    /* error */
                    goto error;
                }
                temp = findOperandType(jmpLabel);

                if(temp != NULL && strcmp(temp,"#") != 0 && strcmp(temp,"r") != 0)
                {
                    operand1 = findOperand(&operandType1,operand1,0);
                    operand2 = findOperand(&operandType2,operand2,0);

                    if((operandType1 == 3 && !operand1) || (operandType2 == 3 && !operand2)) /* make functions for err checks */
                    {
                        /* undefined register */
                        goto error;
                    }

                    if(operandType1 == 0 && !operand1)
                    {
                        /* overflow error */
                        goto error;
                    }

                    if(operandType2 == 0 && !operand2)
                    {
                        /* overflow error */
                        goto error;
                    }

                    if ((operand1 || operand2) && findUserError(operandType1, operandType2,numberOfCommas)) {
                        /* error */
                        goto error;
                    }

                    if((operand1 != NULL && operand2 == NULL) || (operandType1 == 1 && !verifyParameters(operand1)) || (operandType2 == 1 && !verifyParameters(operand2))) /* used to catch edge cases */
                    {
                        /* error, only 1 param found or illegal params */
                        errCode = 10;
                        goto error;
                    }

                    if(writeMachineCode(instructionNumber,instructionCode,operand1,operand2,operandType1,operandType2,jmpLabel,file) == -1)
                    {
                        /* error */
                        goto error;
                    }
                }
                else
                {
                    /* error */
                    errCode = 11;
                    goto error;
                }
            }
        }

        if(label)
        {
            newNode->type = 0;
            newNode->next = NULL;
            newNode->code = NULL;

            if (labelHead)
            { /* loops until the last added node and adds newNode */
                tempNode = labelHead;

                while(tempNode->next)
                {
                    tempNode = tempNode->next;
                }
                tempNode->next = newNode;
            }
            else
            {
                labelHead = newNode;
            }
        }
    }

    return;

    error:
    if(operand1 && operandType1 == 0)
        free(operand1);
    if(operand2 && operandType2 == 0)
        free(operand2);
    if(newNode)
    {
        free(newNode->name);
        free(newNode);
    }
}

/* finds data(.data,.string.entry.extern) and adds it as a new node in the dataHead linked list
 * if entry/extern is found adds it to the entryExternHead linked list
 *
 * @param flag - denotes the type of data
 * @param numberOfCommas - the number of commas in the line
 * @param node - a node that may contain a label in it, NULL if no label was found initially
 *
 * returns -1 on failure
 * returns 0 on success
 */
int storeData(int flag,int numberOfCommas, node *newNode)
{
    char *temp = NULL, *allocateAble = NULL, **allocArr = NULL, *label = NULL;
    int allocated, i, len, exType;
    node *tempNode = NULL;
    allocated = 0, i = 0;

    if(flag == 1)
    { /* data is int */
        if((temp = searchData())) /* get next int */
        {
            do
            {   /* validates that number follows syntax according to instruction and 
                    general rules(such as no spaces after a number)*/
                if(!validateNumber(temp,1)) 
                {
                    errCode = 2;
                    if(allocArr)
                    {
                        i = 0;
                        while(i < allocated)
                        {
                            free(allocArr[i]);
                            i++;
                        }
                        free(allocArr);
                    }
                    return -1;
                }

                /* allocates space for the variable/data */
                if(allocated == 0)
                {
                    allocArr = (char**) malloc (sizeof(char*) * 10);

                    if(!allocArr) {
                        fprintf(stderr, "memory allocation error");
                        free(allocArr);
                        exit(-1);
                    }
                }
                else if(allocated % 10 == 0)
                { /* if more than 10 codes were found we need to realloc and get more space */
                    allocArr = (char **) realloc(allocArr, sizeof(char*) * (10 * (allocated/10+1)));
                    if(!allocArr) {
                        fprintf(stderr, "memory allocation error");
                        exit(-1);
                    }
                }

                if(!(allocateAble = convertToBinary(atoi(temp),14,1))) /* check for overflow */
                {
                    /* error */
                    i = 0;
                    while(i < allocated)
                    {
                        free(allocArr[i]);
                        i++;
                    }
                    free(allocArr);
                    return -1;
                }
                else { /* store variable/data */
                    allocateAble[14] = '\0';
                    allocArr[allocated] = allocateAble;
                    allocated++;
                }
            }
            while ((temp = searchData()));

            /* there should be n-1 commas if there are n datum(3,5,7 has 2 commas) */
            if(numberOfCommas >= allocated) 
            {
                i = 0;
                while(i < allocated)
                {
                    free(allocArr[i]);
                    i++;
                }
                free(allocArr);
                errCode = 23;
                return -1;
            }

            if(!newNode) /* nameless data */
            {
                newNode = (node*)malloc(sizeof(node));
                if(!newNode)
                {
                    /*error*/
                    fprintf(stderr, "memory allocation error");
                    exit(-1);
                }
                newNode->name = NULL;
            }

            newNode->numberOfCodes = allocated;
            newNode->code = allocArr;
            newNode->type = 0;
        }
        else
        {
            /* error */
            errCode = 3;
            return -1;
        }
    }
    else if(flag == 2)
    { /* data = string */

        if(!(temp = searchString(NULL))) /* get string */
        {
            /* error */
            errCode = 4;
            return -1;
        }
        else
        {
            len = strlen(temp);
            i = 0;
            do {

                if(allocated == 0)
                {
                    allocArr = (char**) malloc (sizeof(char*) * 10);
                    if(!allocArr) {
                        fprintf(stderr, "memory allocation error");
                        free(allocArr);
                        exit(-1);
                    }
                }
                else if(allocated % 10 == 0)
                {/* if more than 10 codes were found we need to realloc and get more space */
                    allocArr = (char **) realloc(allocArr, sizeof(char*) * (10 * (allocated/10+1)));
                    if(!allocArr) {
                        fprintf(stderr, "memory allocation error");
                        free(allocArr);
                        exit(-1);
                    }
                }

                allocateAble = convertToBinary(temp[i],14,0);
                allocArr[allocated] = allocateAble;
                allocated++;
                i++;
            }
            while (len >= i);

            if(!newNode) { /* nameless data */
                newNode = (node *) malloc(sizeof(node));
                if (!newNode) {
                    /*error*/
                    fprintf(stderr, "memory allocation error");
                    exit(-1);
                }
                newNode->name = NULL; 
            }
            newNode->numberOfCodes = allocated;
            newNode->code = allocArr;
            newNode->type = 0;
        }
    }
    else if(flag == 3 || flag == 4)
    { /* entry or extern label */
        temp = strtok(NULL, "' '\t\v\r\f\n");

        if(!validateLabel(temp,1) || !validateEnding())
        {
            /* error */
            errCode = 1;
            return -1;
        }
        else
        {
            if(flag == 3)
            {
                exType = 1;
                if(!validateEntryExtern(temp,1))
                {
                    errCode = 17;
                    return -1;
                }
            }
            else
            {
                exType = 2;
                if(!validateEntryExtern(temp,2))
                {
                    errCode = 18;
                    return -1;
                }
            }

            if(!newNode) {
                newNode = (node *) malloc(sizeof(node));
                if (!newNode) {
                    /*error*/
                    fprintf(stderr, "memory allocation error");
                    exit(-1);
                }
            }
            else
            {
                free(newNode->name);
                newNode->name = NULL;
            }

            newNode->type = exType;
            label = (char*) malloc(sizeof(char)*(strlen(temp)+1));

            if(!label)
            {
                fprintf(stderr,"memory allocation error");
                exit(-1);
            }

            memcpy(label,temp,strlen(temp));
            label[strlen(temp)] = '\0';
            newNode->name = label;
            newNode->pc = 0;
            newNode->code = NULL;
        }
    }
    else
    {
        /* error */
        errCode = 6;
        return -1;
    }

    newNode->next = NULL;
    
    /* attaches new data to its linked list */
    if(flag < 3)
    {
        if(dataHead)
        {
            tempNode = dataHead;

            while(tempNode->next)
            {
                tempNode = tempNode->next;
            }
            tempNode->next = newNode;
        }
        else
        {
            dataHead = newNode;
        }
    }
    else
    {
        if(entryExternHead)
        {
            tempNode = entryExternHead;

            while(tempNode->next)
            {
                tempNode = tempNode->next;
            }
            tempNode->next = newNode;
        }
        else
        {
            entryExternHead = newNode;
        }
    }
    return 0;
}

/*
 * prints the data part of the file at the end
 * @param file - the file to write to
 */
void addData(FILE *file)
{
    node *tempNode;
    int i;
    tempNode = dataHead;

    while (tempNode) {

        if (tempNode->name != NULL) { /* assigns the correct pc to a data that had a label in its definition */
            tempNode->pc = PC;
        }

        for (i = 0; i < tempNode->numberOfCodes; i++) { /* writes the data stored in the node */
            fprintf(file, "0%d    %s\n", PC, tempNode->code[i]);
            DC++;
        }
        tempNode = tempNode->next;

        /* can be commented back in if limiting the memory/PC is a need for the project but the instructions were unclear */
        /* if ((PC) > 256) {
             errCode = -1;
             fprintf(stderr,"Error! all memory used too many instructions given!");
             break;
         } */

    }
}

/*
 * Verifies that each entry was defined
 * returns 0 if an entry that was not defined is found
 * returns 1 if every entry was defined
 *
 * example of propery definition:
 * .entry Label
 * Label: ...
 */
int verifyEntryUse()
{
    node *tempNode,*tempNode2,*tempNode3;

    tempNode = entryExternHead;

    while (tempNode) {
        if (tempNode->type == 1) {

            tempNode2 = labelHead;

            while (tempNode2) {
                if (strcmp(tempNode2->name, tempNode->name) == 0)
                    break;

                tempNode2 = tempNode2->next;
            }

            if ((!tempNode2 && labelHead) || !labelHead) {
                tempNode3 = dataHead;

                while (tempNode3) {
                    if (tempNode3->name && strcmp(tempNode3->name, tempNode->name) == 0)
                        break;

                    tempNode3 = tempNode3->next;
                }
            }

            if (!tempNode2 && !tempNode3) {
                fprintf(stderr, "Error: entry: %s is never used", tempNode->name);
                errCode = -1;
                return 0;
            }
        }
        tempNode = tempNode->next;
    }

    return 1;
}

/*
 * writes formatted line to machine code file
 *
 * @param instructionNumber - number between 0-13 that represents the opcode number
 * @param instructionCode - the name of the instruction
 * @param operand1 - the first operand in the instruction if it exists
 * @param operand2 - the second operand in the instruction if it exists
 * @param operandType1 - a number 0/1/3 representing the type of operand number/label/register
 * @param operandType2 - a number 0/1/3 representing the type of operand number/label/register
 * @param jmpLabel - if it is a jmp/jsr/bne instruction we need a label to jump to
 * @param file - the file to be written into
 *
 * returns 1 if successful
 * returns -1 if an error has occured
 */
int writeMachineCode(int instructionNumber,char* instructionCode, char* operand1,char* operand2, int operandType1, int operandType2, char *jmpLabel,FILE *file)
{
    char *opType1Binary = NULL, *opType2Binary = NULL;
    
    if(operand1)
        opType1Binary = convertToBinary(operandType1,2,0);

    if(operand2)
        opType2Binary = convertToBinary(operandType2,2,0);

    switch (instructionNumber)
    {
        case 0: /* mov */case 2: /* add */ case 3:/*sub */
            if (operandType2 > 0) /* if its a number it cant have a number moved into it */
            {
                fprintf(file, "0%d    0000%s%s%s00", PC, findOpCode(instructionCode), opType1Binary,opType2Binary);

                IC++;
            }
            else
            {
                /* error */
                if(opType1Binary)
                    free(opType1Binary);
                if(opType2Binary)
                    free(opType2Binary);
                errCode = 12;
                return -1;
            }
            break;

        case 1: /* cmp */
            if (fprintf(file, "0%d    0000%s%s%s00", PC, findOpCode(instructionCode),
                        opType1Binary,
                        opType2Binary) < 0) {
                /* error */
            }
            IC++;

            break;

        case 4: /* not */ case 5: /* clr */ case 7: /* inc */ case 8: /* dec */ case 11: /* red */
            if (operandType1 > 0) {
                if (fprintf(file, "0%d    0000%s00%s00", PC, findOpCode(instructionCode),
                            opType1Binary) < 0) {
                    /* error */
                }
                IC++;

               /* switch (operandType1) {
                    case 1:
                        fprintf(file, "\n0%d    %s", PC, operand1);
                        break;
                    case 3:
                        fprintf(file, "\n0%d    %s00000000", PC, operand1);
                        break;
                }
                IC++; */
            }
            else
            {
                /* error */
                if(opType1Binary)
                    free(opType1Binary);
                if(opType2Binary)
                    free(opType2Binary);
                errCode = 13;
                return -1;
            }
            break;

        case 6: /* lea */
            if ((operandType1 == 1 && (operandType2 == 3 || operandType2 == 1)))
            {
                if (fprintf(file, "0%d    0000%s%s%s00", PC, findOpCode(instructionCode),
                            opType1Binary,opType2Binary) < 0) {
                    /* error */
                }
                IC++;

                /* fprintf(file, "\n0%d    %s", PC, operand1);
                IC++;

                switch (operandType2) {
                    case 1:
                        fprintf(file, "\n0%d    %s", PC, operand2);
                        break;
                    case 3:
                        fprintf(file, "\n0%d    000000%s00", PC, operand2);
                        break;
                }
                IC++; */
            }
            else
            {
                if(opType1Binary)
                    free(opType1Binary);
                if(opType2Binary)
                    free(opType2Binary);

                if(operandType1 != 1)
                {
                    errCode = 13;
                }
                else if(operandType2 == 0)
                {
                    errCode = 12;
                }
                return -1;
            }
            break;

        case 12: /* prn */
            if (fprintf(file, "0%d    0000%s00%s00", PC, findOpCode(instructionCode),
                        opType1Binary) < 0) {
                /* error */
            }
            IC++;

            /* switch (operandType1) {
                case 0:
                    fprintf(file, "\n0%d    %s00", PC, operand1);
                    break;
                case 1:
                    fprintf(file, "\n0%d    %s", PC, operand1);
                    break;
                case 3:
                    fprintf(file, "\n0%d    000000%s00", PC, operand1);
                    break;
            }
            IC++; */
            break;

        case 9:/* jmp */ case 10:/* bne */ case 13: /* jsr */
            if(operand1 == NULL && operand2 == NULL)
            {
                if (fprintf(file, "0%d    0000%s000100", PC, findOpCode(instructionCode)) < 0)
                {/* error */}
                IC++;
                fprintf(file, "\n0%d    %s", PC, jmpLabel);
                IC++;
            }
            else
            {
                if (fprintf(file, "0%d    %s%s%s001000", PC, opType1Binary,opType2Binary, findOpCode(instructionCode)) < 0)
                {/* error */}
                IC++;

                fprintf(file, "\n0%d    %s", PC, jmpLabel);
                IC++;
            }
            break;
    }

    writeOperands(operandType1,operandType2,operand1,operand2,file);

    /* frees allocated strings */
    if(operand1 && operandType1 == 0)
        free(operand1);
    if(operand2 && operandType2 == 0)
        free(operand2);
    if(opType1Binary)
        free(opType1Binary);
    if(opType2Binary)
        free(opType2Binary);

    return 1;
}

/*
 * writes pc number and operand binary code to file
 *
 * @param operand1 - the first operand in the instruction if it exists
 * @param operand2 - the second operand in the instruction if it exists
 * @param operandType1 - a number 0/1/3 representing the type of operand number/label/register
 * @param operandType2 - a number 0/1/3 representing the type of operand number/label/register
 * @param file - a file to write to
 */
void writeOperands(int operandType1, int operandType2, char *operand1, char *operand2, FILE *file)
{
    if(operandType1 > -1) {

        switch (operandType1) {
            case 0:
                fprintf(file, "\n0%d    %s00", PC, operand1);
                break;
            case 1:
                fprintf(file, "\n0%d    %s", PC, operand1);
                break;
            case 3:
                if (operandType2 == 3) {
                    fprintf(file, "\n0%d    %s%s00", PC, operand1, operand2);
                } else fprintf(file, "\n0%d    %s00000000", PC, operand1);
                break;
        }
        IC++;

        if (!(operandType1 == 3 && operandType2 == 3) && operandType2 != -2) {
            switch (operandType2) {
                case 0:
                    fprintf(file, "\n0%d    %s00", PC, operand2);
                    break;
                case 1:
                    fprintf(file, "\n0%d    %s", PC, operand2);
                    break;
                case 3:
                    fprintf(file, "\n0%d    000000%s00", PC, operand2);
                    break;
            }
            IC++;

        }
    }
}

/*
 * finds the operand type and returns the operand as a string
 *
 * type decides if its a jmp/jsr/bne instruction type = 0 -> jmp/jsr/bne type 1 -> the rest of the command types
 *
 * @param result - is a pointer to the operand type integer variable
 * @param buff - a string containing the operand
 * @param type - explained above
 *
 * if result is equal to -1 there is an error
 * if result is equal to 0 returns a number in binary
 * if result is equal to 3 returns a register number in binary
 * if result is equal to 1 returns a label name
 */
char* findOperand(int *result, char buff[], int type)
{
    char *temp, *operand;
    int i,len;
    *result = -1;

    if(buff != NULL || (operand = searchOperand(type))) {
        if(!buff)
            temp = findOperandType(operand);
        else
        {
            operand = buff;
            temp = findOperandType(buff);
        }

        if (!temp) {
            /* error */
            return NULL;
        }
        else if (strcmp(temp, "r") == 0) {/* register */
            *result = 3;
            if(type == 0 && strlen(operand) > 2)
            {
                errCode = 10;
                return NULL;
            }
            return findRegister(operand);
        }
        else if(strcmp(temp,"d") == 0) /* checks if the register that is found is illegaly defined */
        {
            errCode = 21;
            return NULL;
        }
        else if (strcmp(temp, "#") == 0) {/* number */
            if(type != 0)
            {
                i = 0;
                len = strlen(operand);

                while(operand && isspace(*operand) && len > i)
                {
                    operand++;
                    i++;
                }
            }
            operand++;
            if (validateNumber(operand,type))
            {
                *result = 0;
                return convertToBinary(atoi(operand), 12,1);
            } else {
                /* illegal dec of number */
                errCode = 14;
                return NULL;
            }
        }
        else
        {/* Label */
            if(type != 0)
            {
                i = 0;
                len = strlen(operand);

                while(operand && isspace(*operand) && len > i)
                {
                    operand++;
                    i++;
                }
            }

            
            *result = 1;
            return operand;
            
        }
    }
    
    /* error */
    return NULL;
}

/*
 * checks for errors in the operand types found and the number of commas in the line
 * @param operandType1 - the operand type of the first operand found
 * @param operandType2 - the operand type of the first operand found
 * @param numberOfCommas - the number of commas in the line
 * returns 1 if an error is found
 * returns 0 if there are no errors
 */
int findUserError(int operandType1, int operandType2, int numberOfCommas)
{
    if(operandType1 == -1)
    {
        /* error - send to error function */
        errCode = 8;
        return 1;
    }
    else if(operandType2 == -1)
    {
        /* error */
        errCode = 9;
        return 1;
    }
    else if(numberOfCommas > 1 || (operandType2 < 0 && numberOfCommas > 0))
    {/* if we have more than 1 comma in an instruction we have an error, or if we have more than 1 comma and we couldn't find a second paramater or dont need one */
        errCode = 23;
        return 1;
    }
    else if(numberOfCommas < 1 && operandType2 > -1)
    {/* this catches edge cases for jmp/jsr/bne when there are commas but we dont have params */
        errCode = 24;
        return 1;
    }
    else if(!validateEnding())
    {
        /* error */
        errCode = 19;
        return 1;
    }

    return 0;
}

/*
 * checks if the ':' character is in the line and its not a part of a string that belongs to a .string declaration
 * @param buff - the line to be checked
 * returns 1 if ':' is found within the line
 * returns 0 if it is not found
 */
int checkLabel(char buff[])
{
    int len,i;
    if(buff) {
        len = strlen(buff);
        for (i = 0; i < len; i++) {
            if (buff[i] == ':' || buff[i] == '"')
                break;
        }

        if (buff[i] == ':')
            return 1;
    }
    return 0;
}

/*
 * checks if a line is a data instruction or a normal instruction
 * @param buff - the line excluding a an initial label decleration if it exists
 * @param instruction - a pointer to a string that will save an instruction if its not a data instruction
 * returns the data name or saves the name of the instruction in param instruction
 */
char* checkData(char buff[], char **instruction)
{
    char* temp;

    if(!buff)
         temp = strtok(NULL,"' '\t\v\r\f\n"); /* continues from the last point we stopped from with strtok */
    else temp = strtok(buff, "' '\t\v\r\f\n");

    if(temp != NULL) {
       /* for (i = 0; i < strlen(temp) && temp[i] != '.'; i++) {} */ /* skips to '.' if it exists */
        if (temp[0] != '.') { /* if the first character in the string is not a '.' we have a normal instruction */
            *instruction = temp; /* assigns the instruction name to the pointer intended for it */
            return NULL;
        } else return temp + 1; /* return the string without the '.' character */
    } else return NULL; /* no command was given return NULL*/
}

/*
 * finds the string after .string declaration and verifies that its valid, returns the string if its valid otherwise returns NULL
 * @param buff - the string that comes after a .string decleration
 * returns NULL if the string is invalid
 * returns the string if its valid
 */
char* searchString(char buff[])
{
    int i, flag = 0, indexOfLastQuotation, len;
    char *temp;

    if(!buff){
        temp = strtok(NULL, "");
    }
    else temp = buff;

    if(temp) {
        len = strlen(temp);

        for (i = 0; i < len; i++) {
            if (isspace(temp[i])) { /* remove trailing spaces */
                temp++;
                len--;
                i--;
            } else if (temp[i] == '"') { /* found opening quotation mark */
                temp++;
                break;
            } else {
                return NULL;
            }
        }

        if (i == len)
            return NULL;

        for (i = 0; i < len; i++) {
            if (temp[i] == '"') { /* found closing quotation mark */
                flag = 0;
                indexOfLastQuotation = i;
            } else if (temp[i] != '"' && !isspace(temp[i]) && temp[i] != '\0') {
                flag++;
            }
        }

        if (flag > 0)
            return NULL;
        else {
            temp[indexOfLastQuotation] = '\0';
            return temp;
        }
    } return NULL;
}

/*
 * finds the next operand
 * if type = 0 -> finds jmp/jsr/bne operand
 * if type = 1 -> otherwise
 *
 * NOTE: I've changed the function and commented out the old use of it and now the if statement is meaningless
 */
char* searchOperand(int type)
{
    if(!type) { /* jmp/jsr/bne */
        return strtok(NULL, "','\n"); /* strtok(NULL,"','' '\t\v\f\r\n"); */
    }
    else return strtok(NULL,"','\n");  /* \t\v\f\r were removed */
}

/*
 * finds the next data
 */
char* searchData()
{
    return strtok(NULL,"','\n");
}

/*
 * finds the instruction

char* checkInstruction(char buff[])
{
    if(!buff)
        return strtok(NULL,"' '\t\r\v\f");
    else return strtok(buff,"' '\t\r\v\f");
}
*/

/*
 * finds bne,jmp,jsr parameters
 * @param param1 - pointer to the first operand of a jsr/jmp/bne command
 * @param param2 - pointer to the first operand of a jsr/jmp/bne command
 * returns 0 if there are no parameters
 * returns -1 if an error has occured
 */
int findParameters(char **param1, char **param2)
{
    if((*param1 = strtok(NULL,"','"))) /* checks if a comma exists and if it doesnt we either have no params or an error depending on if param1 is null*/
    {
        if((*param2 = strtok(NULL,"')'"))) /* checks if a right paranthesis exists and if it doesnt we either have no closing paranthesis or no second paramater*/
        {
            if((*param2)[strlen(*param2)-1] == '\n') /* if we didnt find a paranthesis our next character should be a next line char */
            {
                errCode = 15;
                return -1;
            }
            return 0;
        }
        else /* if param2 is null that means we have no second param and thus we have an error */
        {
            errCode = 16;
            return -1;
        }
    }
    else return 0; /* there are no params and we return 0 */
}

/*
 * checks if a label param of a jsr/jmp/bne function is properly defined
 * @param buff - the parameter string
 * returns 0 if not defined correctly
 * returns 1 if defined correctly
 */
int verifyParameters(char buff[])
{
    int length,i;

    if(buff) {
        length = strlen(buff);

        for (i = 0; i < length; i++) {

            if(!isalnum(buff[i]))
                return 0;
        }
    }
    return 1;
}

/*
 * checks the type of delecaration that was made and returns an integer between 0-4
 *
 * @param type - the decleration type's name
 *
 * returns:
 * 0 -> undefined
 * 1 -> data
 * 2 -> string
 * 3 -> entry
 * 4 -> extern
 */
int validateDataType(char type[])
{
    if(type) {
        if (strcmp(type, "data") == 0) {
            return 1;
        } else if (strcmp(type, "string") == 0) {
            return 2;
        } else if (strcmp(type, "entry") == 0) {
            return 3;
        } else if (strcmp(type, "extern") == 0) {
            return 4;
        }
    }
    return 0;
}

/*
 * checks if the label was declared according to instructions/rules
 * @param buff - the string that will be checked
 * @param type - 0 -> jmp/jsr/bne type instruction 1-> others
 * @return 0 -> error 1 -> success
 */
int validateLabel(char buff[],int type)
{
    int length,i,flag;
   

    if(buff) {
        length = strlen(buff);

        for (i = 0; i < NUM_OF_OPCODES; i++) { /* check that the label is not a saved op name */
            if (strcmp(buff, oparr[i].name) == 0)
                return 0;
        }

        for (i = 0; i < NUM_OF_REGS; i++) { /* check that the label is not a saved reg name */
            if (strcmp(buff, regarr[i]) == 0)
                return 0;
        }

        /* check that the label is not a saved name */
        if (strcmp(buff, "string") == 0 || strcmp(buff, "data") == 0 || strcmp(buff, "entry") == 0 || strcmp(buff,"extern") == 0) 
            return 0;

        if (length < 31) { /* a label can be 30 letters long at most */
            flag = 0;

            if(type == 0) {
                for (i = 0; i < length; i++) {
                    if (isalpha(buff[i])) { /* makes sure we've found at least one letter */
                        flag++;
                    } else if ((isspace(buff[i]) && flag) || (!flag && isdigit(buff[i])) ||
                               ispunct(buff[i])) /* if we've found a space after finding a letter the label is not properly defined */
                        return 0;
                }
            }
            else
            {
                for (i = 0; i < length; i++) {
                    if(isalnum(buff[i]) && flag == 2)
                    {
                        return 0;
                    }
                    else if(isalpha(buff[i])) { /* makes sure we've found at least one letter */
                        flag = 1;
                    } else if ((isspace(buff[i]) && flag)) /* if we've found a space after finding a letter the label is not properly defined */
                        flag = 2;
                    else if((!flag && isdigit(buff[i])) || ispunct(buff[i]))
                        return 0;
                }
            }

            if(flag > 0) /* check that we've found at least one letter */
            {
                return 1;
            } else return 0;
        }
    }

    return 0;
}

/*
 * finds the operand type by scanning the given string buff
 *
 * @param buff - the operand
 *
 * returns "#" if the operand is a number
 * returns "d" if its a register that is not properly defined
 * returns "r" if its a register that is properly defined
 * returns a label name if properly defined
 * returns NULL if a label name is not properly defined
 */
char *findOperandType(char buff[])
{
    char *temp;

    if(buff) {
        temp = findRegister(buff); /* checks if its a register */

        if (strchr(buff, '#')) /* a number */
            return "#";
        else if (temp && strcmp(temp,"d") == 0){ /* checks if the register that is found is illegaly defined */
            return "d";
        }
        else if(temp) /* if we found a properly defined register we return "r" */
        {
            return "r";
        }
        else { /* if its non of the options we check if its a label and check that its properly defined */
            if (validateLabel(buff,1))
                return buff;
        }
    }
    return NULL;
}

/*
 * compares opcode names with the given string
 * @param buff - the instruction's name
 * returns a number between 0-15 from the instruction list
 * otherwise returns -1
 */
int getInstructionType(char buff[])
{
    if(buff != NULL) {
        int i;

        for (i = 0; i < NUM_OF_OPCODES; i++) {
            if (strcmp(buff, oparr[i].name) == 0) {
                return i;
            }
        }
    }
    return -1;
}

/*
 * scans buff and finds the register number in binary that corresponds to it
 * @param buff - the string to be compared
 * returns the register number in binary
 */
char *findRegister(char buff[])
{
    int len, i;

    if(buff) {

        len = strlen(buff);
        i = 0;
        while(isspace(*buff) && i < len)
        {
            buff++;
            i++;
        }

        if (buff[0] != 'r' || !isdigit(buff[1]))
            return NULL;
        else /* changes that were made */
        {
            len -= i;
            /* starts from 2 to skip first 2 letters because we've confirmed the beginning is properly defined */
            for(i = 2; i < len; i++)
            {
                if(!isspace(buff[i]))
                    return NULL;
            }
        }

        switch (atoi(++buff)) {
            case 0:
                return "000000";
            case 1:
                return "000001";
            case 2:
                return "000010";
            case 3:
                return "000011";
            case 4:
                return "000100";
            case 5:
                return "000101";
            case 6:
                return "000110";
            case 7:
                return "000111";
            default:
                return "d";
        }
    }

    return NULL;
}

/*
 * compares param command with opcode list
 * if the command is not in the list of commands returns NULL
 * otherwise returns the binary of the instruction/opcode
 * @param command - the command to be compared
 */
char *findOpCode(char command[])
{
    int i;

    if(command) {
        for (i = 0; i < NUM_OF_OPCODES; i++) {
            if (strcmp(command, oparr[i].name) == 0)
                return oparr[i].binary;
        }
    }
    return NULL;
}
/*
 * translates a decimal to binary
 * dec is the number to be translated to binary and n is the number of digits the string will have at most
 * overflow param determines if overflow of the number should be taken into consideration 1-> it can 0 -> it can't
 *
 * @param dec - a number
 * @param n - the length of the binary string to be returned
 * @param overflow - determines if overflow should be taken into consideration
 * returns NULL if overflow happens and the overflow flag is on
 */
char* convertToBinary(int dec, int n, int overflow)
{
    char *temp;
    int i,mask,limit;
    limit = pow(2,n-1);

    if(overflow && ((dec > (limit-1)) || (dec < (-1 * limit)))) {
        errCode = 20;
        return NULL;
    }

    temp = (char *) malloc(sizeof(char) * n + 1);

    if (!temp) {
        /* error */
        fprintf(stderr, "memory allocation error");
        exit(-1);
    }

    i = 0;
    mask = 1 << (n - 1);

    while (mask) {
        if (mask & dec) {
            temp[i] = '1';
        } else {
            temp[i] = '0';
        }

        i++;
        mask >>= 1;
    }
    temp[i] = '\0';

    return temp;

}

/* checks if there is any non white space chars after the last parameter 
*  @return 0 if it is not valid, 1 if it is
*/
int validateEnding()
{/* double check if \n is needed */
    if(strtok(NULL,"' '\t\r\v\f\n"))
        return 0;
    else return 1;
}

/*
 * checks if there are any clashing entry/extern labels by looping through entry/extern list
 * type 0 -> normal label
 * type 1 -> entry
 * type 2 -> extern
 * @param labelName - a name of a label
 * @param type - explained above
 */
int validateEntryExtern(char labelName[], int type)
{
    node *temp;
    temp = entryExternHead;

    if(type != 0) { /* if its not a normal label we compare it to all of the list */
        while (temp) { /* checks if the label already exists in entry/extern list*/
            if (strcmp(temp->name, labelName) == 0) {
                return 0;
            }

            temp = temp->next;
        }
    }
    else
    {
        temp = entryExternHead;

        while(temp)
        {
            if(temp->type == 2)
            {
                if(strcmp(labelName,temp->name) == 0)
                {
                    errCode = 22;
                    return 0;
                }
            }
            temp = temp->next;
        }
    }

    if(type != 1) /* extern or normal label */
    { /* checks if an extern label is clashing with a non entry label*/
        temp = labelHead;

        while(temp)
        {
            if(strcmp(temp->name,labelName) == 0)
            {
                if(!type)
                    errCode = 22;
                return 0;
            }

            temp = temp->next;
        }

        temp = dataHead;

        while(temp)
        {
            if(temp->name && strcmp(temp->name,labelName) == 0 )
            {
                if(!type)
                    errCode = 22;
                return 0;
            }

            temp = temp->next;
        }
    }


    return 1;
}

/*
 * checks if number is valid according to syntax
 * @param buff - a string to validate
 * @param type - determines which function will be using it
 * returns 0 if its not valid
 * return 1 if it is
 */
int validateNumber(char buff[],int type)
{
    int len,i, flag;
    flag = 0;

    if(buff != NULL) {
        len = strlen(buff);

        if(type == 1) { /* for non jmp/jsr/bne */

            i = 0;
            while(isspace(*buff) && i < len)
            {/* get rid of initial spaces */
                buff++;
                i++;
            }

            len = strlen(buff);

            for (i = 0; i < len; i++) {/* loop checks if we've found a number and if its defined properly */
                if (isdigit(buff[i]) && flag <= 1)
                    flag = 1;
                else if ((i == 0) && (buff[i] == '-' || buff[i] == '+')) {}
                else if (isspace(buff[i]) && flag) {}
                else return 0;
            }

            if(flag == 0)
                return 0;
        }

        if(type == 0) { /* used for jmp/jsr/bne */
            for (i = 0; i < len; i++) {
                if (!isdigit(buff[i])) {  /* if we find anything that is not a digit that means we have a space or a non integer and thus jmp/jsr/bne is not defined correctly */
                    if (i == 0 && (buff[i] == '-' || buff[i] == '+')) /* first character can be a negative sign or a plus sign */
                        {}
                        else return 0;
                }
                else flag++; /* found a number */
            }

            if(flag == 0)
                return 0;
        }
        return 1;
    }

    return 0;
}

/*
 * checks if the line is empty
 * @param buff - a string to be checked
 */
int checkEmptyLine(char buff[])
{
    if(buff) {
        int len, i;
        len = strlen(buff);

        for (i = 0; i < len; i++)
            if (!isspace(buff[i]))
                return 0;
    }
    return 1;
}

/*
 * finds the number of commas in the string buff
 * @param buff - a string to be checked
 */
int findNumberOfCommas(char buff[])
{
    int len,i,count;
    len = strlen(buff);
    count = 0;

    for(i = 0; i < len; i++)
    {
        if(buff[i] == ',')
            count++;
    }

    return count;
}

/*
 * frees dataHead,labelHead,entryExternHead linked lists
 */
void freeAllLists()
{
    freeList(dataHead);
    freeList(labelHead);
    freeList(entryExternHead);
    dataHead = NULL;
    labelHead = NULL;
    entryExternHead = NULL;
}

/*
 * frees a given list
 * @param head - the linked list to be freed
 */
void freeList(node *head)
{
    node *temp;
    int i;
    i = 0;

    while(head)
    {
        temp = head;
        head = head->next;
        if(temp->name != NULL)
            free(temp->name);
        if(temp->code != NULL) {
             while(i < temp->numberOfCodes) {
                 free(temp->code[i]);
                 i++;
             }
            free(temp->code);
        }
        free(temp);
        i = 0;
    }

    head = NULL;
}

/*
 * prints the error based on errCode value
 * if errorCode = 0 it means we had no error
 * if errorCode = -1 it means we didnt have an error this time around but an error was found before
 *
 * 1- Illegal dec of label name
 * 2- data has a non integer in its listed data
 * 3- data was defined without any data
 * 4- string declared with no given string or no opening/closing quotation marks
 * 5- Too many parameters given to
 * 6- line starts with . but is not data/string/entry/extern
 * 7- Instruction was not from the 16 instruction types
 * 8- Operand1 was illegaly defined
 * 9- Operand2 was illegaly defined
 * 10- parameters for jmp/jsr/bne are illegaly defined
 * 11- jmp/jsr/bne's first param was not a label
 * 12- Illegal second operand
 * 13- Illegal first operand
 * 14- Number not defined correctly - example: #a1
 * 15- No closing paranthesis for jmp/jsr/bne instruction
 * 16- given 1 parameter to jmp/jsr/bne instruction
 * 17- clashing entry and extern label
 * 18- label clashing
 * 19- illegal chars after last param
 * 20- overflow
 * 21- invalid register number
 * 22- label definition already exists
 * 23- too many commas in data dec
 *
 * @param linesRead - the line that the error happened on
 * @param fileName - the name of the file
 */
void printError(int linesRead, char* fileName)
{
    if(errCode != 0 && errCode != -1)
    {
        fprintf(stderr, "Error in file %s: Line %d - ", fileName,linesRead);
        switch (errCode) {
            case 1:
                fprintf(stderr, "Illegal decleration of label name\n");
                break;
            case 2:
                fprintf(stderr,"data has to be an integer");
                break;
            case 3:
                fprintf(stderr,".data delecration given with missing data");
                break;
            case 4:
                fprintf(stderr,".string deceleration given with no string defined after it or a missing quotation mark");
                break;
            case 5:
                fprintf(stderr, "Too many parameters given");
                break;
            case 6:
                fprintf(stderr,"Undefined data type");
                break;
            case 7:
                fprintf(stderr,"Undefined instruction");
                break;
            case 8:
                fprintf(stderr,"Operand1 is illegaly defined");
                break;
            case 9:
                fprintf(stderr,"Operand2 is illegaly defined");
                break;
            case 10:
                fprintf(stderr, "illegal deceleration of parameters in a jmp/jsr/bne instruction");
                break;
            case 11:
                fprintf(stderr, "First parameter of a jmp/jsr/bne instruction has to be a label");
                break;
            case 12:
                fprintf(stderr, "Illegal second parameter");
                break;
            case 13:
                fprintf(stderr, "Illegal first parameter");
                break;
            case 14:
                fprintf(stderr,"illegal deceleration of number");
                break;
            case 15:
                fprintf(stderr,"no closing paranthesis for paramaters passed by a jmp/jsr/bne instruction");
                break;
            case 16:
                fprintf(stderr,"Only one parameter given to a jmp/jsr/bne instruction");
                break;
            case 17:
                fprintf(stderr,"clashing decleration of entry/current file label with an extern label OR two entry declerations were made");
                break;
            case 18:
                fprintf(stderr,"clashing between extern label deceleration with an entry label or a current file label decelration OR two extern declerations were made");
                break;
            case 19:
                fprintf(stderr, "illegal characters after last parameter");
                break;
            case 20:
                fprintf(stderr,"overflow");
                break;
            case 21:
                fprintf(stderr,"invalid register number please use registers 0-7");
                break;
            case 22:
                fprintf(stderr,"Label definition already exists");
                break;
            case 23:
                fprintf(stderr,"Illegal decleration of data, too many commas");
                break;
        }
        fprintf(stderr,"\n");
        errCode = -1;
        return;
    }
    else return;
}
/* returns NULL if the next node does not exist, otherwise returns the next node */
node *getNext(node *node)
{
    if(node)
        return node->next;
    else return NULL;
}

/* returns -1 if the node is NULL, otherwise returns the node's PC*/
int getPC(node *node)
{
    if(node)
        return node->pc;
    else return -1;
}

/* returns NULL if the node does not exist, otherwise returns the node's name */
char *getName(node *node)
{
    if(node)
        return node->name;
    else return NULL;
}

int getType(node *node)
{
    if(node)
        return node->type;
    else return -1;
}
