#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pre_assembler.h"
#include "first_transition.h"

/* NOTE:
 * every function assumes max length of line is 82 because according to instructions a line is 81 chars long including the /n
 * and because a string requires null termination i've set it to 82, fgets also reads upto n-1 chars if it hasn't found \n because it puts null termination at the nth spot
 * */

int main(int argc, char *argv[]) {
    FILE* fd;
    FILE* ES;
    int args;
    char *sourceFile = NULL;

    args = 1;

    while(args < argc) {
        sourceFile = (char*) calloc((strlen(argv[args]) + 4), sizeof(char)); /* allocate space for source file name */

        if(!sourceFile)
        {
            fprintf(stderr,"memory allocation error");
            exit(-1);
        }

        memcpy(sourceFile,argv[args],strlen(argv[args]));


        if (!(fd = fopen(strcat(sourceFile, ".as"), "r"))) {
            printf("File %s can't be opened",sourceFile);
            free(sourceFile);
            args++;
        }
        else
        {
            ES = pre_assembler(fd,argv[args]);
            fclose(fd);
            first_transition(ES,argv[args]);

            args++;
            free(sourceFile);
        }
    }

    return 0;
}
