typedef struct mcr mcr;

FILE* pre_assembler(FILE* file, char *argName);

int findCheckString(char string[], char find[], int n);

int checkMacro(char string [], mcr *macroList[], int n);

char *getMacroName(char string [], int index);

void addMCRtoTable(FILE *file, mcr *macroArr [], int *numberOfMCRS, char buff[], int i);

void freeMCRList(mcr *macroArr[], int numberOfMacros);

int isEmptyLine(char buff[]);