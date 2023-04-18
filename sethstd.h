/*
This file contains commonly used functions which are
not available through standard libraries and are used
across many assignments and labs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
sfree checks if a pointer is allocated
if it is, free it. Then set the pointer
to NULL for future verification
*/
#define sfree(ptr) \
if(ptr){\
    free(ptr);\
}\
ptr = NULL;

void* errorCheck(void* ptr, const char* message, int exitCode);
int errNum(int num, const char* message, int exitCode);
char* tricat(const char* prefix, const char* middle, const char* suffix);
void* smalloc(unsigned int size, const char* failmsg, int errnum);
void* scalloc(unsigned int count,unsigned int size,const char* msg,int errnum);
void* srealloc(void* ptr, unsigned int size, const char* failmsg, int errnum);
