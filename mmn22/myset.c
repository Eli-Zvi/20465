#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "set.h"

#define SIZE_OF_ARRAY 100

void stop();
int getCommandType(char *command);
int getSet();
char *getCommand();
void findError(int errorNumber);
int *getNumberList(int *n);
void initialOutput();
void initialize(set *set_arr[]);
void freeSets(set *set_arr[]);
int validateEnding();
int scanInput(char input[],int *commacount);

char *sets[] = {"SETA","SETB","SETC","SETD","SETE","SETF"};

char *commands[] = {"stop", "read_set", "print_set", "union_set", "intersect_set", "sub_set", "symdiff_set"};

/*
 * main class of the program
 * calls for printing functions, derives and analyzes commands and calls for functions in set.c
 *
 * commandnumber codes: -1/-2: undefined command name 0: stops program 1-6:commands[1-6] 7: Missing param 8:error msg was already displayed
 * commacount is used for tracking the number of commas, legal command type 1 has n+1 of them command type 2 has 0 and the rest have 2
 */
int main()
{
    /* n is the number of integer in the array sent to read_set, i is used for looping. endValidation is a boolean if the end of the command only contains white spaces */
    int commandNumber, endValidation, n, i,commacount = 0;
    int setNumber[3] = {0,0,0};
    char *command;
    int *numberList = NULL;
    set *set_array[6];
    
    initialOutput(); /* display user msg */
    
    initialize(set_array);
    
    for(;;)
    { /* add check to each function to see if the pointer returned is null, if it is break out of switch */
        printf("\nPlease type a command\n");

        command = getCommand();

	if(command == NULL)
	{
	   free(command);
           freeSets(set_array);
           findError(0);
	}
	
        if(scanInput(command,&commacount))
        {
            commandNumber = getCommandType(command);

            if(commandNumber == 0) { /* if stop is entered will stop function, checks validity of ending before */
                if(validateEnding()) {
                    printf("stop\n");
                    free(command);
                    freeSets(set_array);
                    stop();
                } else
                {
                    findError(7); /* if the ending is not valid displays an error msg */
                }
            }

            if (commandNumber != -2 && commandNumber != 0)
            { /* error codes for -2 and 0 were displayed before, thus we can continue to the next command */
                if (commandNumber != -1) /* displays error code if the command is invalid */
                {
                    setNumber[0] = getSet(); /* every command left has at least 1 set parameter */

                    if (commandNumber >= 3) { /* if there are 3 set parameters obtain the other 2 */
                        setNumber[1] = getSet();
                        setNumber[2] = getSet();

                        if (setNumber[1] == -1 || setNumber[2] == -1)
                            commandNumber = 7;
                        else if(commacount != 2)
                        {
                            commandNumber = 8;
                            findError(10);
                        }

                    }

                    if (setNumber[0] == -1)
                        commandNumber = 7;

                    if(setNumber[0] == -2)
                        commandNumber = 8;

                    if(commandNumber == 1) {
                        numberList = getNumberList(&n);
                        if(!numberList)
                            commandNumber = 8;
                    }

                    endValidation = validateEnding();

                    if (endValidation || commandNumber > 6)
                    {
                        switch (commandNumber) {
                            case 1:
                                if (n + 1 == commacount) {
                                    read_set(set_array[setNumber[0]], numberList, n);
                                    printf("\noperation done successfully\n");
                                }
                                else if(n + 1 > commacount) findError(10);
                                else findError(9);
                                break;

                            case 2:
                                if(commacount == 0) {
                                    print_set(set_array[setNumber[0]]);
                                    printf("\noperation done successfully\n");
                                } else findError(9);
                                break;

                            case 3:
                                union_set(set_array[setNumber[0]], set_array[setNumber[1]], set_array[setNumber[2]]);
                                printf("\noperation done successfully\n");
                                break;

                            case 4:
                                intersect_set(set_array[setNumber[0]], set_array[setNumber[1]],
                                              set_array[setNumber[2]]);
                                printf("\noperation done successfully\n");
                                break;

                            case 5:
                                sub_set(set_array[setNumber[0]], set_array[setNumber[1]], set_array[setNumber[2]]);
                                printf("\noperation done successfully\n");
                                break;

                            case 6:
                                symdiff_set(set_array[setNumber[0]], set_array[setNumber[1]], set_array[setNumber[2]]);
                                printf("\noperation done successfully\n");
                                break;

                            case 7:
                                printf("Missing parameter\n");
                                break;

                            case 8: /* error code thrown by another function */
                                break;

                            default:
                                findError(-1); /* default error code */
                                break;
                        }
                    } else findError(7);
                } else printf("Undefined command name\n");
            }
        }
        if(numberList)
        {
           free(numberList);
           numberList = NULL;
        }
        free(command);
        command = NULL;
        commacount = 0;
        n = 0;
        for(i = 0; i < 3; i++)
        {
            setNumber[i] = 0;
        }
    }

    return 0;
}

/*
 * initializes set arrays
 */
void initialize(set *set_arr[])
{
    int i;
    for(i = 0; i < 6; i++)
        set_arr[i] = create_set();
}

void freeSets(set *set_arr[])
{
    int i;
    for(i = 0; i < 6; i++)
    {
        free(set_arr[i]);
        set_arr[i] = NULL;
    }
}

/* supposed to take user input to display help but did not want to mess up the use of files so i commented it out*/
void initialOutput()
{
    

    printf("Hello, welcome to My Set!\n");
    printf("In this program you can create and perform operations on sets!\n");
    printf("If you want to see the list of commands and their formats with an explanation enter 0\notherwise enter 1\n");
/* 
   int c;
   while(((c = getchar()) != '0' && c != '1' )&& c != EOF)
    {
        if(c != '\n')
            printf("Invalid input please enter 0 or 1 based on the instructions given above\n");
    }

    if(c == EOF)
    {
    	printf("Error: Attempt to end program using EOF, program will now terminate");
    	exit(0);
    }
    
    if(c == '0')
    {*/
        printf("List of Commands:\n");
        printf("SETA/SETA/SETC is a set given by the user from the list of sets (SETA-SETF), command names have to be lower letter and set names have to be capitalized\n");
        printf("When typed in the commands have to match the formats(excluding white spaces) as listed below otherwise an error message will be displayed\n");
        printf("\nread_set SETA, list of integers - assigns the list of ints to the given set, integers need to be separated by commas\n");
        printf("print_set SETA\n");
        printf("union_set SETA,SETB,SETC - SETC = SETA U SETB\n");
        printf("intersect_set SETA,SETB,SETC - SETC = SETA ∩ SETB\n");
        printf("sub_set SETA,SETB,SETC - SETC = SETA - SETB\n");
        printf("symdiff_set SETA,SETB,SETC - SETC = (SETA U SETB) - (SETA ∩ SETB)\n");
        printf("stop - will stop the program\n");
   /* } c = getchar(); get rid of \n that is in the stream */ 
}

/* gets user input */
char *getCommand()
{
    int length = 0, size = SIZE_OF_ARRAY;
    char c;
    char *input = (char*)malloc(sizeof(char) * size);

    if(!input)
    {
        free(input);
        printf("Memory allocation error");
        return NULL;
    }

    while((c = getchar()) != '\n' && c != EOF) {

        if (length % size == 0 && length != 0) {
            size += SIZE_OF_ARRAY;
            input = (char *) realloc(input, sizeof(char) * size);

            if (!input) {
                free(input);
                printf("Memory allocation error");
                return NULL;
            }
        }

        input[length] = c;
        length++;
    }

    input[length] = '\0';
    printf("\nYour input is: %s\n",input);

    if(c == EOF)
    {
        return NULL;
    }

    return input;
}

/* gets command type checks for illegal commas and illegal chars in command name */
int getCommandType(char *command)
{
    int i;
    char *temp = strtok(command," ' '\t\r\v");

    if(temp) {
        for(i = 0; i < strlen(temp); i++)
        {
            if(temp[i] == ',') {
                findError(9);
                return -2;
            }
            else if(!isalpha(temp[i]) && temp[i] != '_')
            {
                findError(8);
                return -2;
            }
        }
        for (i = 0; i < 7; i++)
            if (strcmp(temp, commands[i]) == 0) {
                return i;
            }
    }


    return -1;
}

/* finds set and checks for illegal comma and undefined set name */
int getSet()
{
    char *temp = strtok(NULL," ' '\t\r\v,");
    int i;

    if(temp)
    {
        if(temp[0] == ',')
        {
            findError(9);
            return -2;
        }

        if(strlen(temp) == 4)
        {
            for (i = 0; i < 6; i++)
                if (strcmp(temp, sets[i]) == 0)
                {
                    return i;
                }
            findError(6);
            return -2;
        }
    }

    return -1;
}

/* scans for number list and its size, passes back an array of numbers */
int *getNumberList(int *n)
{
    int count = 0, size = 50;
    int num;

    char *token = strtok(NULL, " ' '\t\r\v,");
    int *ret = (int*)calloc(size,sizeof(int));

    if(!ret)
    {
        free(ret);
        printf("Memory allocation error");
        return NULL;
    }

    while(token != NULL && token[0] != '-')
    {
        num = strtol(token,&token,10);

	if(isalpha(token[0]))
        {
            free(ret);
            findError(3);
            return NULL;
        }
        
        if(num < 0 || num > 127)
        {
            findError(4);
            free(ret);
            return NULL;
        }


        if(count % size == 0 && count != 0)
        {
            size += 50;
            ret = (int*)realloc(ret,size * sizeof(int));
            if(!ret)
            {
                free(ret);
                printf("Memory allocation error");
                return NULL;
            }
        }

        ret[count] = num;
        count++;

        token = strtok(NULL, " ' '\t\r\v,");
    }

    if(token == NULL) /* no -1 at the end */
    {
        findError(5);
        free(ret);
        return NULL;
    }

    if(strcmp(token,"-1") != 0) { /* the negative number found is not -1 */
        findError(4);
        free(ret);
        return NULL;
    }

    *n = count;

    return ret;
}

/* checks for illegal chars and consecutive commas/ multiple negative numbers */
int scanInput(char input[], int *commacount)
{
    enum {Number = 0, Comma = 1};
    char c;
    int i = 0, flag = Number, negatives = 0,underscores = 0, commandCommaCheck = 0,length = strlen(input);

    while(i < length)
    {
        c = input[i];

        if(!isdigit(c) && !isalpha(c))
        {
            if(isspace(c))
            {

            }
            else if(c == '_' && underscores == 0)
            {
                underscores++;
            }
            else if(c == ',' && flag == Comma)
            {
                findError(2);
                return 0;
            }
            else if(c == ',')
            {
                if(commandCommaCheck)
                {
                    findError(9);
                    return 0;
                }
                *commacount += 1;
                flag = Comma;
            }
            else if(c == '-')
            {
                if(negatives == 0 && i+1 < length && input[i+1] == '1')
                    negatives++;
                else if(negatives == 1 || (i+1 < length && isdigit(input[i+1])))
                {
                    findError(4);
                    return 0;
                }
                else
                {
                    findError(8);
                    return 0;
                }
            }
            else
            {
                findError(8);
                return 0;
            }
        }
        else
        {
            if(c == 't')
                commandCommaCheck = 1;
            else commandCommaCheck = 0;
            flag = Number;
        }
        i++;
    }

    if(flag == Comma)
    {
        findError(7);
        printf("May be caused by list of members not being terminated correctly if the command was read_set or text after the last parameter or a missing parameter\n");
        return 0;
    }

    return 1;
}
/*
 * finds the errors
 * error #0 : attempt to end the program with EOF, ends program and sends an error message
 * error #1 : illegal space in integer list
 * error #2 : consecutive commas
 * error #3 : non integer error in list of numbers
 * error #4 : value out of range
 * error #5 : missing -1 at the end of list of number for a read_set call
 * error #6 : set name is not one of the 7 in command array
 * error #7 : text after end of command
 * error #8 : not a letter/integer error
 * error #9 : comma in a place where it is not supposed to be 
 * error #10: missing comma between parameters
 */
void findError(int errorNumber)
{
    switch(errorNumber)
    {
        case 0:
            printf("Error: Attempt to end input with EOF, ");
            stop();
        case 1:
            printf("illegal space in integer array");
            break;
        case 2:
            printf("Multiple consecutive commas");
            break;
        case 3:
            printf("Invalid set member – not an integer");
            break;
        case 4:
            printf("Invalid set member – value out of range");
            break;
        case 5:
            printf("List of set members is not terminated correctly");
            break;
        case 6:
            printf("Undefined set name");
            break;
        case 7:
            printf("Extraneous text after end of command");
            break;
        case 8:
            printf("Illegal character");
            break;
        case 9:
            printf("Illegal comma");
            break;
        case 10:
            printf("Missing comma");
            break;
        default:
            printf("default error code");
    }
    printf("\n");
}

/* checks if there is any non white space chars after the last parameter */

int validateEnding() 
{
    if(strtok(NULL," ' '\t\r\v\n,"))
        return 0;
    else return 1;
}

void stop()
{
    printf("program will now exit");
    exit(0);
}



        /*if(token[0] != '0' && num == 0)
        {
            findError(3);
            free(ret);
            return NULL;
        }*/
