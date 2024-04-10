#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "permut.h"

#define MAXARGS 3
#define SIZE_OF_ARRAY 100

/*
 * the main function of the program permut.c
 *
 * the program finds permutation by comparing the word given in the argument line and comparing it to substrings
 * of words in the file that is given in the argument line
 *
 * when an empty file is given will display the contents of an empty file (nothing) and the program will print that no permutations were found
 */
int main(int argc, char *argv[])
{
    FILE *fd;
    int length , count = 0;
    char *word1, *word2, *tempWord;

    if(!validateArgs(argc))
        return -1;

    if(!(fd = fopen(argv[1], "r")))
    {
        fprintf(stderr, "Error: File can't be opened\n");
        return -1;
    }

    length = strlen(argv[2]);
    word1 = (char*)malloc(sizeof(char) * length); /* adding 1 more cell to add '\0' at the end of the string */

    if(!word1)
    {
        fprintf(stderr, "Memory allocation error\n");
        return -1;
    }

    memcpy(word1,argv[2], length);

    sortWord(word1,length);

    word2 = getArray(fd);

    if(!word2)
    {
        fprintf(stderr,"Error: File is empty\n");
        free(word1);
        return -1;
    }

    tempWord = strtok(word2, " ' '\t\r\v\n");

    while(tempWord) {
        compareString(word1, tempWord, &count);

        tempWord = getWord();
    }

    if(count == 0)
        printf("no permutations found\n");
    else printf("The number of permutations of the word %s that were found in the file is %d\n", argv[2], count);

    free(word1);
    free(word2);
    return 0;
}

/* scans file and converts its contents into a string that is returned */
char *getArray(FILE *stream)
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

    while(!feof(stream))
    {
        c = fgetc(stream);

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

    if (length % size == 0 && length != 0) {
        size += 1; /* if array is filled we need to add '/0' thus we need to add 1 more cell */
        input = (char *) realloc(input, sizeof(char) * size);

        if (!input) {
            free(input);
            printf("Memory allocation error");
            return NULL;
        }
    }

    if(length != 1) {
        input[length - 1] = '\0'; /* gets rid of EOF at the end of input */
        printf("\nThe contents of the file are: \n%s\n\n", input);
    }
    else /* if we only have the EOF in we have an empty file and we print an error */
    {
        free(input);
        input = NULL;
    }

    fclose(stream);
    return input;
}

/* gets the next word in the file */
char *getWord()
{
    return strtok(NULL, " ' '\t\r\v\n");
}

/*
 * compares the word we are searching the permutations of with one of the words in the file
 * returns 0 if a permutation was not found, otherwise returns the number of permutations found
 */
void compareString(char word1[], char word2[], int *count)
{
    int i, diff, len;
    char *temp;
    i = 0;

    len = strlen(word1);
    temp = (char*)malloc( sizeof(char) * (len + 1));
    temp[len] = '\0';
    diff = strlen(word2) - len;

    while(diff - i >= 0)
    {
        memcpy(temp, word2 + i, len);
        sortWord(temp, len);

        if(memcmp(temp, word1 ,len) == 0)
        {
            printf("%.*s\n", len,word2+i); /* prints the permutation */
            *count = *count + 1;
            i += len;
        } else i++;
    }

    free(temp);
}

/*
 * implements bubblesort on array to sort it out according to ascii values
 */
void sortWord(char word[], int length)
{
    int i,j;

    for(i = 0; i < length - 1; i++)
        for(j = 0; j < length - 1 - i; j++)
        {
            if(word[j] > word[j+1])
            {
                swap(&word[j],&word[j+1]);
            }
        }
}

/*
 * swaps between characters in an array
 */
void swap(char *c, char *c2)
{
    char temp;

    temp = *c;
    *c = *c2;
    *c2 = temp;
}

/*
 * validates that the number of the args we got is the number we needed
 */
int validateArgs(int argc)
{
    if(argc == MAXARGS)
        return 1;
    else if(argc > MAXARGS)
        fprintf(stderr, "Error: Too many arguments given\n");
    else
        fprintf(stderr, "Error: Too few arguments given\n");
    return 0;
}