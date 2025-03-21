#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "pre_assembler.h"
#define MAX_LENGTH 82 /* MAX LENGTH is 82 because of null termination character */

struct mcr{
    char *name;
    char **string;
    int n;
};

/*
 * functions scans a line, and proceeds to either print it, add it as a new macro definition or print a macro name that is found
 * @param file - file to be read out of(source file)
 * @param argName - the name of the file
 */
FILE* pre_assembler(FILE* file, char *argName)
{
    FILE *ES; /* extended_source_file is the file to be returned, it is the new source file */
    char *macroFileName, *temp;
    char buff[MAX_LENGTH];
    mcr **macroArr = NULL;
    int numberOfMCRS, flag, i,count,argNameLength, linesRead;

    numberOfMCRS = 0;
    flag = 0; /* flag = 3 -> new macro found flag = 4 -> comment or whitespace flag = 5 -> max length of line exceeded */
    i = 0;
    count = 0;
    linesRead = 1;
    argNameLength = strlen(argName);

    /* allocate space for macro file name */
    macroFileName = (char*) calloc((argNameLength + 4), sizeof(char));

    if(!macroFileName)
    {
        fprintf(stderr,"memory allocation error");
        exit(-1);
    }

    memcpy(macroFileName,argName,argNameLength);

    ES = fopen(strcat(macroFileName,".am"), "w+");

    while(!feof(file))
    {
        if (fgets(buff, MAX_LENGTH, file) != NULL)
        { /*puts(buff);*/}
        else break;

        if(strlen(buff) == MAX_LENGTH-1 && buff[MAX_LENGTH-2] != '\n')
        { /* MAX_LENGTH-1 = 81, buff[MAX_LENGTH-2] is the last character before the null termination */
            fprintf(stderr, "Error in file %s: Line %d is more than 81 characters\n", argName, linesRead);

            /* flush the buffer until the next line character is found or until the length of the line is less than 81*/
            do {
                fgets(buff,MAX_LENGTH,file);
            }
            while(strlen(buff) == MAX_LENGTH-1 && buff[MAX_LENGTH-2] != '\n');

            flag = 5; /* flag tells scanner to skip the line and allows us to recheck the next line for errors */
        }

        if(buff[0] == ';' || isEmptyLine(buff)) /* if its a comment line or an empty line we skip it */
            flag = 4;
        else {
            if (strstr(buff, "mcr")) /* checks if the string mcr exists in the line */
            {
                while (flag < 3 && buff[i] != '\n' && buff[i] != EOF && i < 81) {
                    if (isalnum(
                            buff[i])) { /* if a character was found check that its the right one in the order of "mcr" */
                        if (!flag && buff[i] == 'm') {}
                        else if (flag == 1 && buff[i] == 'c') {}
                        else if (flag == 2 && buff[i] == 'r') {}
                        else break;
                        flag++;
                    } else if (flag &&
                               !isalpha(buff[i])) /* make sure that there arent spaces or numbers in the mcr dec */
                        break;

                    i++;
                }
            }
        }

        if(flag == 3)
        {

            if(numberOfMCRS == 0)
            {
                macroArr = (mcr**) malloc (sizeof(mcr*) * 10);

                if(!macroArr) {
                    fprintf(stderr, "memory allocation error");
                    free(macroArr);
                    exit(-1);
                }
            }
            else if(numberOfMCRS % 10 == 0)
            {
                macroArr = (mcr**) realloc(macroArr, sizeof(mcr*) * (10 * (numberOfMCRS/10+1)));
                if(!macroArr) {
                    fprintf(stderr, "memory allocation error");
                    free(macroArr);
                    exit(-1);
                }
            }

            addMCRtoTable(file, macroArr, &numberOfMCRS, buff, i);
        }
        else if(flag < 4 && (i =checkMacro(buff, macroArr, numberOfMCRS)) >= 0) /* if a macro name is found print it */
        { /* assumes there is a space between a macro and other instructions */

            /* go over the buff string and print anything else on the line that isnt the macro */
            temp = strtok(buff," \t\v\r\f\n");

            while(temp)
            {
                if(strcmp(temp,macroArr[i]->name) == 0)
                {
                    while(count < macroArr[i]->n){ /* write the macro to the macro file */
                        fputs(macroArr[i]->string[count], ES);
                        count++;
                    }
                    count = 0;
                }
                else fprintf(ES,"%s ",temp); /* will print what comes before and after the macro name */

                temp = strtok(NULL,"' '\t\v\r\f\n");
            }
        }
        else
        {
            if(flag != 5) {
                if (!fputs(buff, ES)) {
                    /* print error in the file and close the program possibly(?) */
                }
            }
        }

        flag = 0;
        i = 0;
        linesRead++;
    }

    freeMCRList(macroArr,numberOfMCRS);

    free(macroFileName);
    return ES;
}

/*
 * returns the name of the mcr
 * @param string is the string to be checked
 * @param index is the index of the space after the string "mcr"
 */
char *getMacroName(char string [], int index)
{
    int i,length,count;
    char *temp;

    if(string && index < strlen(string)) {
        i = index;
        length = strlen(string);
        count = 0;

        while (string[i] != '\n' && i < length) {
            if (isspace(string[i])) { /* skip spaces */
                if (count > 0) { /* if a space is found after a letter we've found the macro name */
                    temp = (char*) malloc(sizeof(char) * (count+1)); /* allocate memory for the name */
                    temp[count] = '\0'; /* replaces \n with null termination */
                    return memcpy(temp, string + i - count, count); /* store the substring containing the name in temp and return a pointer to it */
                }
            } else if (isalpha(string[i]) || isdigit(string[i])) {  /* could be replaced by isalnum, checks if we've found a letter or digit and increments the letter count */
                count++;
            }
            i++;
        }

        temp = (char *) malloc(sizeof(char) * (count+1)); /* allocate memory for the name */
        temp[count] = '\0'; /* replaces \n with null termination */
        return memcpy(temp, string + i - count, count); /* store the substring containing the name in temp and return a pointer to it */
    } return NULL;
}

/*
 * checks if a macro out of the list of macros is found in the line read
 * @param string is the line
 * @param macroList is the list of macros
 * @param n is the total number of macros found/stored so far
 */
int checkMacro(char string [], mcr *macroList[], int n)
{
    int i;
    char* temp;

    if(string && macroList) {
        i = 0;

        while (i < n) {

            temp = strstr(string, macroList[i]->name); /* checks if macroList[i]->name is in the line */

	    /* if temp is null macroList[i]->name is not a substring, otherwise we check that temp 
     		and macroList[i]->name are identical */
            if ((temp && strncmp(temp, macroList[i]->name, strlen(macroList[i]->name)) == 0))
                if(strlen(temp) == strlen(macroList[i]->name) || isspace(temp[strlen(macroList[i]->name)]))
                    return i;

            i++;
        }
    }
    return -1;
}

/*
 * searches for the string find in the string and verifies that no chars are after it
 * @param string the string to search in
 * @param find the string to be found in the string
 * @param n is the number of letters in the macro name + the number of letters in "mcr"(3)
 */
int findCheckString(char string[], char find[], int n)
{
    int i,count;
    if(string && find) {
        i = 0;
        count = 0;

        if(strstr(string, find)) /* checks if the string find is in the string at all */
        {
            while (i < 81 && string[i] != '\n') {
                if (isdigit(string[i]) || isalpha(string[i]))
                    count++;

                if (count > n) /* if we've found more characters than expected then we have an error */
                    return -1;
                i++;
            }
            return 1;
        }
    }
    return 0;
}

/* when mcr instruction is found, finds its name, and reads lines in the source file and stores them in macroArr in position numberOfMCRS+1
 * until endmcr is found
 * @param file is the file we read out of
 * @param macroArr is the array of macros
 * @param numberOfMCRS is the number of macros found
 * @param i is the index of the first space after "mcr"
 */
void addMCRtoTable(FILE *file, mcr *macroArr [], int *numberOfMCRS, char buff[], int i)
{
    char *temp = NULL, **stringArr = NULL; /* will be used for memory allocation for the two fields in the mcr struct */
    mcr *tempMCR = NULL;
    int numberoflines, flag;

    numberoflines = 0;
    temp = getMacroName(buff, i+1);

    if(temp && !isEmptyLine(temp)) /* check that temp is not NULL or not only white space */
    {
        tempMCR = (mcr*) malloc (sizeof(mcr)); /* create a new struct */
        tempMCR->name = temp; /* assign the name we've found to it */
    }
    else
    {
        /* error */
        fprintf(stderr,"macro defined with no name\n");
        freeMCRList(macroArr,*numberOfMCRS);
        exit(-1);
    }

    if(findCheckString(buff,"\n", strlen(temp) + 3) > 0) /* check that there's only white space after macro definition */
    {}
    else
    {
        fprintf(stderr,"macro definition line has characters after name definition");
        freeMCRList(macroArr,*numberOfMCRS);
        exit(-1);
    }

    if (fgets(buff, MAX_LENGTH, file) != NULL)
    {/*puts(buff);*/}
    
    while(!(flag = findCheckString(buff, "endmcr", 6)))
    { /* as long as we didnt find endmcr line we continue saving the lines inside the macro definition */

        if(strlen(buff) == MAX_LENGTH-1 && buff[MAX_LENGTH-2] != '\n')
        { /* MAX_LENGTH-1 = 81, buff[MAX_LENGTH-2] is the last character before the null termination */
            fprintf(stderr, "Error in macro, a macro line contains more than 81 characters\n");
            /* flush the buffer until the next line character is found or until the length of the line is less than 81*/
            do {
                fgets(buff,MAX_LENGTH,file);
            }
            while(strlen(buff) == MAX_LENGTH-1 && buff[MAX_LENGTH-2] != '\n');

            buff[0] = ';';
        }

        if(buff && buff[0] != ';') {

            if (numberoflines == 0) {
                stringArr = (char **) malloc(sizeof(char *) * 10);
            } else if (numberoflines % 10 == 0) {
                stringArr = (char **) realloc(stringArr, sizeof(char *) * (10 * (numberoflines / 10 + 1)));

                if (!stringArr) {
                    fprintf(stderr, "memory allocation error");
                    free(macroArr);
                    free(temp);
                    exit(-1);
                }
            }

            temp = (char *) malloc(sizeof(char) * (strlen(buff)+1));

            if(!temp)
            {
                free(macroArr);
                free(temp);
                fprintf(stderr, "memory allocation error");
                exit(-1);
            }

            strcpy(temp, buff); /* copies the line we've found into temp */
            stringArr[numberoflines] = temp; /* saves temp in the numberoflines index of stringArr */
            numberoflines++;
        }

        if (fgets(buff, MAX_LENGTH, file) != NULL) {
            {/*puts(buff);*/}
        }
    }

    if(stringArr != NULL) { /* not sure if an empty macro is regarded as an error */
        tempMCR->string = stringArr;
    } else fprintf(stderr,"Warning: macro %s's body is empty\n",tempMCR->name);
    tempMCR->n = numberoflines;
    macroArr[*numberOfMCRS] = tempMCR; /* assign the macro to the macro list */
    *numberOfMCRS = *numberOfMCRS + 1;
}

/* checks if a line is empty, returns 0 if its not, 1 if it is */
int isEmptyLine(char buff[])
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
 * frees all mcrs
 */
void freeMCRList(mcr *macroArr[], int numberOfMacros)
{
    int i,j;
    j = 0;
    for(i = 0; i < numberOfMacros; i++)
    {
        free(macroArr[i]->name);
        while(j < macroArr[i]->n)
        {
            free(macroArr[i]->string[j]);
            j++;
        }
        free(macroArr[i]->string);
        free(macroArr[i]);
        j = 0;
    }
    free(macroArr);
}
