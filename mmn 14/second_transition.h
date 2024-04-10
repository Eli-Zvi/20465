#include "data_structures.h"

char *lineToSpecialForm(char buff[]);

/*void secondTransition(node *dataHead, node *labelHead, node *entryExternHead,int IC, int DC, FILE *file, char *argName);*/

int verifyAndPrintLabel(char *buff,int lineNumber,node *labelHead, node *dataHead, node *entryExternHead, FILE *finalFile, FILE *externFile,int *externCount);

int addToEntryFile(FILE *entryFile, node *entryExternHead, node *labelHead, node *dataHead);
