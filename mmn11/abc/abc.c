#include <stdio.h>
#include <string.h>
#define MAXSIZE 80

int size = 0;

void shortenString(char string [])
{

    char comparedChar;
    int streak = 1;
    int i,j,current = 0;
    int length;
    length = strlen(string);

    for(i = 0; i < length;)
    {
        comparedChar = string[i];

        string[current] = comparedChar;
        current++;

        /* checks the length of the sequence of letters if it exists */
        if(comparedChar >64 && comparedChar < 123)
        {
            for (j = i + 1; j < length; j++)
            {
                if (comparedChar + streak == string[j])
                    streak++;
                else break;
            }
        }

        /* if our streak/sequence is 3 letters or more assign the char at position i+1
        to be a hyphen and the last letter of the sequence at position i+ 2this could be done using strcopy */
        if(streak > 2) {
            i = i + streak;
            string[current++] = '-';
            string[current++] = string[i-1];

        }else i=i+1;

        streak = 1;
    }
    
    string[current] = '\0';
}

int main()
{
    char string[MAXSIZE];
    
    printf("Please type in the string\n");
    
    while(scanf("%c",&string[size]) != EOF)
    {
    	size++;
    }

   
    if(size != MAXSIZE) /* assigns the end of the string */
    	string[size-1] = '\0'; 	
    	
    printf("\nYour string is:%s\n",string);
    
    shortenString(string);
    
    printf("\nShortened string:%s\n",string);
    
    return 0;
}
