/*
This file contains implementations for the functions
delcared in sethstd.h
*/


#include "sethstd.h"

/*
errorCheck confirms that a
pointer was allocated
*/
void* errorCheck(void* ptr, const char* message, int exitCode){

    /*validate that ptr was intialized*/
    if(ptr == NULL){
        perror(message);
        exit(exitCode);
    }

    return ptr;
}

/*
errNum checks the result of a
system call and verifies that 
the call worked
*/
int errNum(int num, const char* message, int exitCode){

    /*validate that ptr was intialized*/
    if(num == -1){
        perror(message);
        exit(exitCode);
    }

    return num;
}

/*
tricat combines 3 strings
of arbitrary length and 
returns the full, null terminated
string
*/
char* tricat(const char* prefix, const char* middle, const char* suffix){
    int startIndex = 0;
    int middleIndex = 0;
    int endIndex = 0;
    char* final = calloc(strlen(prefix)+strlen(suffix)+strlen(middle)+1, 
    sizeof(char));
    errorCheck(final, "failed to start tricatination", -1);

    for(startIndex = 0; startIndex < strlen(prefix); startIndex++){
        final[startIndex] = prefix[startIndex];
    }

    for(middleIndex = startIndex; middleIndex < strlen(middle)+startIndex; 
    middleIndex++){
        final[middleIndex] = middle[middleIndex-startIndex];
    }

    for(endIndex = middleIndex; endIndex < strlen(suffix)+middleIndex; 
    endIndex++){
        final[endIndex] = suffix[endIndex-middleIndex];
    }
    
    final[endIndex] = '\0';
    return final;
}

/*
smalloc creates, verifies, and
returns a newly allocated pointer
*/
void* smalloc(unsigned int size, const char* failmsg, int errnum){
    void* mem = malloc(size);
    if(!mem){
        perror(failmsg);
        exit(errnum);
    }
    return mem;
}

/*
scalloc creates, initializes to 0,
verifies, and returns a newly 
allocated pointer
*/
void* scalloc(unsigned int count,unsigned int size,const char* msg,int errnum){
    void* mem = calloc(count, size);
    if(!mem){
        perror(msg);
        exit(errnum);
    }
    return mem;
}

/*
smalloc reallocates, verifies, and
returns a pointer
*/
void* srealloc(void* ptr, unsigned int size, const char* failmsg, int errnum){
    void* mem = realloc(ptr, size);
    if(!mem){
        perror(failmsg);
        exit(errnum);
    }
    return mem;
}


