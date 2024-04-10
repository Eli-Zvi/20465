#include <stdio.h>
#include <limits.h>

unsigned int my_rottate(unsigned int a, int b);
void printAll(unsigned int);

int main()
{
    unsigned int a;
    int b;

    printf("Please type in a number between 0 and %u\n",UINT_MAX);
    scanf("%u",&a);
    printf("The number you typed in is: %u\n",a);

    printf("\nPlease type in the number of rotations you want to make, if the number is negative the rotations will be to the left\n");
    scanf("%d",&b);
    printf("The number of rotations are: %d\n",b);
    
    printAll(a);
    a = my_rottate(a,b);
    printf("\nRotation succesful, the new number will be displayed in the different bases\n");
    printAll(a);

    return 0;
}

/* both rotateBitsRight and rotateBitsLeft use bitwise or to place bits that overflowed in their post-rotation position by "sending" them to the beginning (or end depending on the function used) of the binary sequence and then moving them b bits back towards their pre-rotation position*/

unsigned int rotateBitsRight(unsigned int a, int b)
{
    return ((a >> b) | (a << (sizeof(unsigned int)*8-b)));
}

unsigned int rotateBitsLeft(unsigned int a, int b)
{
    return ((a << b) | (a >> (sizeof(unsigned int)*8-b)));
}

void printBinary(unsigned int a)
{
    unsigned int mask = 1 << (sizeof(int)*8-1);

    printf("\nthe number in binary is: ");

    while(mask)
    {
        (mask&a)?putchar('1'): putchar('0');
        mask>>=1;
    }
    
}

void printAll(unsigned int a)
{
    printBinary(a);
    printf("\nthe number in octal representation: %o",a);
    printf("\nthe number in hexadecimal representation: 0x%X",a);
    printf("\nthe number in decimal: %u\n",a);
}

/* Checks if the rotation is to the right or left and sends it to the correct rotation function */
unsigned int my_rottate(unsigned int a, int b)
{   
   return ((b > 0) ? rotateBitsRight(a,b):rotateBitsLeft(a,-b));
}




