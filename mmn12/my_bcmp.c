#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SIZE 512

int my_bcmp(const void*, const void*, int);
void getInput(int*, int*, int*, char*);
void printUserInput(char*, int, int, int);
void getIntegerInput(int *);

int main()
{
    char string[MAX_SIZE];
    int len = 0, index1 = 0, index2 = 0;
    
    getInput(&len, &index1, &index2, string);

    
    if(!my_bcmp(string+index1, string+index2, len))
        printf("\n\nThe bytes are equal\n");
    else printf("\n\nThe bytes are not equal\n");

    return 0;
}

/* gets input from user and checks if its valid */
void getInput(int *len, int *index1, int *index2, char *string)
{
    int length;
    
    printf("All integers should be between 0-511 otherwise the program will be terminated\n");
    printf("if no integer is given the program will treat it as 0\n");
    printf("if the sum of the length and one of the indexes is bigger than the length of the string the program will be terminated\n");
    printf("String input should be 512 characters long at most, any characters after will be ignored\n");

    printf("\nPlease type in the number of bytes you want to compare\n");
    getIntegerInput(len); 

    printf("\nPlease type in the first index\n");
    getIntegerInput(index1);

    printf("\nPlease type in the second index\n");
    getIntegerInput(index2);

    printf("\nPlease type in a string\n");
    fgets(string,MAX_SIZE,stdin);

    length = strlen(string);
    
    if(string[length-1] == '\n') /* we do not check the last byte in the array */
    	length--;
    
    if(length == 0)
    {
    	printf("\nNo string given\n");
    	exit(0);
    }
    
    if(*len + *index1 - 1 >= length || *len + *index2 - 1 >= length) 
    {/*my_bcmp checks the bytes from index1/index2 upto index1(or index 2)+len-1 and length is the index that is either out of bounds,has /0 in it or /n and thus it is no longer part of the string*/
        printf("\nBytes to be checked are out of bounds\n");
        exit(0);
    }   
    
    printUserInput(string, *len, *index1, *index2);
}

/* gets integer input from user while checking for blank spaces such as tabs,spaces and next line */
void getIntegerInput(int *p)
{
    int i = 0, count = 0, c;/* count is used to keep track of the number of digits in a number, i used to check if the integer given by the user is 0*/
     
    
    while((c = getchar()) != EOF && c != '\n')
    {
    
    	if(!isdigit(c) && !(c == ' ' || c == '\t'))
        { /* if it is not a blank space or a digit, there is a non integer given*/
            printf("\nError, non integer input given\n");
            exit(0);
        }
        
        if(!isdigit(c) && (count > 0 || i > 0))
        {/* if we've found a digit and now we've found a non digit, we are done reading input for the current integer */
            break;
        }

        if(isdigit(c))
        {
            if(!(count == 0 && c == '0'))
            {/* if the number starts with a 0 we do not count the 0s */
            
                c = c - '0'; /* convert the character into its integer form */

                if(count == 0)
                {/* if its the first digit we add it in and raise the count by 1 */
                    *p += c;
                    count++;
                }
                else 
                {/*if a number is already stored in p then we need c to be the ones and the rest of the digits to move to the right, we do this by multiplying p by 10 */
                    *p = *p*10 + c; 
                    count++;
                }
            } else i++; /* if we've found 0 as the first digit we will increase i */
        }

        if(count >= 4)
        { /* if there are more than 3 digits to the number its bigger than the max size of the array */
            printf("\nError, integer is too large\n");
            exit(0);
        }
    }
}

/* prints user input */
void printUserInput(char *string, int len, int index1, int index2)
{
    int i = 0;
    printf("\nThe number of bytes to be checked are:%d", len);
    printf("\nThe first index is:%d", index1);
    printf("\nThe second index is:%d", index2);
    printf("\nThe string is:%s\n",string);

    printf("The bytes that will be compared are:\n");

    printf("#1:");
    
    while(i != len)
    {
        printf("%c", string[i + index1]);
        i++;
    }

    printf("\n#2:");

    i = 0;
    while(i != len)
    {
        printf("%c", string[i + index2]);
        i++;
    }

}

int my_bcmp(const void *b1, const void *b2, int len)
{
    char *p1 = (char*) b1;
    char *p2 = (char*) b2;

    while(len != 0)
    {
        if(*p1 != *p2)
            return 1;
        p1++;
        p2++;
        len--;
    }

    return 0;
}
