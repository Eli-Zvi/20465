#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "set.h"
#define SIZE_OF_BYTE 8
#define NUMBER_OF_CHARS 16
#define SIZE_OF_SET SIZE_OF_BYTE * NUMBER_OF_CHARS

struct set {
    unsigned char elements[NUMBER_OF_CHARS];
};

void read_set(set *s, int arr[], int n)
{
    int i;

    emptySet(s);

    for(i = 0; i < n; i++)
    {
          set_bit_on(s,arr[i]);
    }
}

void print_set(set *s)
{
    int i, count = 0;

    for(i = 0; i <= SIZE_OF_SET; i++)
    {
        if(is_bit_on(s,i))
        {
            if(count == 0)
                printf("The set is: \n");
            else if(count % 16 == 0)
                printf("\n");

            printf("%d ",i);
            count++;
        }
    }

    if(count == 0)
        printf("The set is empty\n");
    else printf("\n");
}

void union_set (set *s1, set *s2, set *target)
{
    int i;

    set *temp = create_set();

    for(i = 0; i < SIZE_OF_SET; i++)
    {
        if(is_bit_on(s1,i) || is_bit_on(s2,i))
            set_bit_on(temp,i);
    }

    memcpy(target,temp,SIZE_OF_SET);
    free(temp);
}

void intersect_set(set *s1, set *s2, set *target)
{
    int i;

    set *temp = create_set();

    for(i = 0; i < SIZE_OF_SET; i++)
    {
        if(is_bit_on(s1,i) && is_bit_on(s2,i))
            set_bit_on(temp,i);
    }

    memcpy(target,temp,SIZE_OF_SET);
    free(temp);
}

void sub_set(set *s1, set *s2, set *target)
{
    int i;

    set *temp = create_set();

    for(i = 0; i < SIZE_OF_SET; i++)
    {
        if(is_bit_on(s1,i) && !is_bit_on(s2,i))
            set_bit_on(temp,i);
    }

    memcpy(target,temp,SIZE_OF_SET);
    free(temp);
}

void symdiff_set(set *s1, set *s2, set *target)
{
    set *temp = create_set();
    set *temp2 = create_set();

    union_set(s1,s2,temp);
    intersect_set(s1,s2,temp2);
    sub_set(temp,temp2,target);

    free(temp);
    free(temp2);
}

int isEmpty(set *s)
{
    int i;
    for(i = 0; i < SIZE_OF_SET; i++)
    {
        if(is_bit_on(s,i))
            return 0;
    }
    return 1;
}

void emptySet(set *s)
{
    int i;
    if(!isEmpty(s)) {
        for (i = 0; i < NUMBER_OF_CHARS; i++) {
            s->elements[i] = 0;
        }
    }
}

void set_bit_on(set *s, int index)
{
    s->elements[index/SIZE_OF_BYTE] |= (1 << index%SIZE_OF_BYTE);
}

int is_bit_on(set *s, int index)
{
    return (s->elements[index/SIZE_OF_BYTE] >> (index%SIZE_OF_BYTE) & 1);
}

set* create_set()
{
    set *temp = (set*) calloc(NUMBER_OF_CHARS, SIZE_OF_BYTE);

    if(!temp)
    {
        free(temp);
        printf("memory allocation failed, program will exit");
        exit(0);
    }return(temp);
}
