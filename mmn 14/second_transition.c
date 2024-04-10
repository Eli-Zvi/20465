#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "second_transition.h"
#define LINE_LENGTH 80

/* NOTE: Similar to the machine code file I manually created spaces instead of using a number in the format as it made it easier to visualize */

void secondTransition(node *dataHead, node *labelHead, node *entryExternHead,int IC, int DC, FILE *file, char *argName)
{
    int lineNumber,argNameLength,error,flag,externCount,entryCount;
    char buff[LINE_LENGTH];
    char *temp, *finalFileName, *extFileName, *entryFileName;
    FILE *finalFile,*externFile,*entryFile;

    error = 0;
    flag = 0;
    externCount = 0;
    entryCount = 0;
    argNameLength = strlen(argName);

    /* allocate space for the 3 file names */
    finalFileName = (char*) calloc((argNameLength + 4), sizeof(char));
    extFileName = (char*) calloc((argNameLength + 5), sizeof(char));
    entryFileName = (char*) calloc((argNameLength + 5), sizeof(char));

    if((!finalFileName || !extFileName || !entryFileName))
    {
        fprintf(stderr,"memory allocation error");
        exit(-1);
    }

    /* copy the name of the file to each string */
    memcpy(finalFileName,argName,argNameLength);
    memcpy(extFileName,argName,argNameLength);
    memcpy(entryFileName,argName,argNameLength);

    /* create and open files with the according file extension */
    finalFile = fopen(strcat(finalFileName,".ob"),"w");
    externFile = fopen(strcat(extFileName,".ext"),"w");
    entryFile = fopen(strcat(entryFileName,".ent"),"w");

    /* print .ob file's format */
    fprintf(finalFile,"Base 10 address Base 2 code\n            %d %d\n",IC,DC);

    while(!feof(file)) {

        if (fgets(buff, LINE_LENGTH, file) != NULL) {
            /* error  */
        }
        else break;

        temp = strtok(buff," "); /* gets the PC of the line */
        lineNumber = atoi(temp); /* converts PC string to number */
        fprintf(finalFile,"%s            ",temp); /* prints the PC in the .ob file */
        temp = strtok(NULL, "' '\t\v\f\r\n"); /* gets the binary or label name in the line */

        if(temp != NULL && (temp[0] == '1' || temp[0] == '0')) { /* if the first character is a 0 or 1 its a label */
            fprintf(finalFile, "%s", lineToSpecialForm(temp));
        }
        else
        {
            if(verifyAndPrintLabel(temp,lineNumber,labelHead,dataHead,entryExternHead,finalFile,externFile,&externCount))
            {}
            else
            { /* the label that was found was not defined in the file, display error */
                fprintf(stderr,"Undefined label: %s\n",temp);
                error = 1;
            }
        }

        flag++;
        if(flag != IC+DC) /* if were at the last line in the file we do not need to write a next line character */
            fprintf(finalFile,"\n");
    }

    entryCount = addToEntryFile(entryFile,entryExternHead,labelHead,dataHead);
    /* close the files */
    fclose(externFile);
    fclose(entryFile);
    fclose(finalFile);

    if(error)
    { /* if we encountered an error we need to remove all the files */
        remove(extFileName);
        remove(entryFileName);
        remove(finalFileName);
        free(extFileName);
        free(entryFileName);
        free(finalFileName);
        return;
    }
    else
    {
        if(!externCount)
        {/* if we didnt find any extern label uses we remove the file */
            remove(extFileName);
        }

        if(!entryCount)
        {/* if we didnt find any entry label uses we remove the file */
            remove(entryFileName);
        }

        /* free the names of the files */
        free(extFileName);
        free(entryFileName);
        free(finalFileName);
        return;
    }
}

/*
 * transforms from binary to special form where 1 = '/' and 0 = '.'
 * @param buff - the binary code string
 * returns the special form string
 */
char* lineToSpecialForm(char buff[])
{
    if(buff != NULL) {
        int i;
        int len = strlen(buff);

        for (i = 0; i < len; i++) {
            if (buff[i] == '1')
                buff[i] = '/';
            else buff[i] = '.';
        }
        return buff;
    }
    return NULL;
}

/*
 * searches for the label buff in the label list, data list and the extern labels in the entry/extern linked list
 * if found in data/label/extern list writes to .ob file(finalFile) if was found in extern label list, adds to the externFile
 *
 * @param buff - label name
 * @param lineNumber - the PC of the line
 * @param labelHead - the head of the label linked list
 * @param dataHead - the head of the data linked list
 * @param entryExternHead - the head of the entry/extern label linked list
 * @param file - the .ob file
 * @param entryFile - the entry file(.ent)
 * @param externFile - the extern file(.ext)
 * @param externCount - pointer to the variable that keeps count of the number of extern label uses
 * @param entryCount - pointer to the variable that keeps count of the number of entry label uses
 *
 * returns 1 if the label was found in one of the linked lists
 * returns 0 if it was not found
 */
int verifyAndPrintLabel(char *buff,int lineNumber,node *labelHead, node *dataHead, node *entryExternHead, FILE *finalFile, FILE *externFile,int *externCount)
{
    char *temp = NULL;
    node *tempNode;
    tempNode = labelHead;

    if(buff) {

        while (tempNode)
        {
            if (strcmp(buff, getName(tempNode)) == 0)
            {
                 temp = convertToBinary(getPC(tempNode),12,0);
                 fprintf(finalFile,"%s/.", lineToSpecialForm(temp));
                 free(temp);
                 return 1;
            }
            tempNode = getNext(tempNode);
        }

        tempNode = dataHead;

        while (tempNode)
        {
            if (getName(tempNode) && strcmp(buff, getName(tempNode)) == 0)
            {
                temp = convertToBinary(getPC(tempNode),12,0);
                fprintf(finalFile,"%s/.", lineToSpecialForm(temp));
                free(temp);
                return 1;
            }
            tempNode = getNext(tempNode);
        }

        tempNode = entryExternHead;

        while(tempNode)
        {
            if(getType(tempNode) == 2)
            {
                if (strcmp(buff, getName(tempNode)) == 0)
                {
                    fprintf(externFile,"%s  %d\n",buff,lineNumber);
                    *externCount = *externCount + 1;
                    fprintf(finalFile, "............./");
                    return 1;
                }
            }
            tempNode = getNext(tempNode);
        }
    }

    return 0;
}

/*
 * finds the PC that the entry label was defined on and saves it in the entry file
 *
 * @param entryFile - entry file to be written to
 * @param entryExternHead - the head of the entry/extern label linked list
 * @param labelHead - the head of the label linked list
 * @param dataHead - the head of the data linked list
 *
 * returns the number of entries found
 */
int addToEntryFile(FILE *entryFile, node *entryExternHead, node *labelHead, node *dataHead)
{
    node *tempEX, *temp;
    int count;
    char *name;
    tempEX = entryExternHead;
    count = 0;

    while(tempEX)
    {
        if(getType(tempEX) == 1) {

            if(count > 0)
                fprintf(entryFile,"\n");

            temp = labelHead;

            while (temp) {
                name = getName(temp);

                if (strcmp(name, getName(tempEX)) == 0) {
                    fprintf(entryFile, "%s   %d", name, getPC(temp));
                    count++;
                    break;
                }
                temp = getNext(temp);
            }

            if((labelHead && !temp) || !labelHead) {

                temp = dataHead;

                while (temp) {
                    name = getName(temp);

                    if (name && strcmp(name, getName(tempEX)) == 0) {
                        fprintf(entryFile, "%s   %d", name, getPC(temp));
                        count++;
                        break;
                    }
                    temp = getNext(temp);
                }
            }
        }

        tempEX = getNext(tempEX);
    }

    return count;
}
