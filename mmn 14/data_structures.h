#include <math.h>

typedef struct node node;

void secondTransition(node *dataHead, node *labelHead, node *entryExternHead,int IC, int DC, FILE *file, char *argName);

node *getNext(node *node);

int getPC(node *node);

char *getName(node *node);

char* convertToBinary(int dec, int n,int overflow);

int getType(node *node);