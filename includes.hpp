#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <ctype.h>
#include <algorithm>
#include <algorithm>

#define MAXMSG 200

#define MAXBUF 1400
#define DEBUG_FLAG 1

#define HANDLELENGTH 100
#define DEFAULTSIZE 100
#define SIZETHRESH 0.5

#define FLAG_NEWCLIENT 1
#define FLAG_ACCEPTCLIENT 2
#define FLAG_REJECTCLIENT 3
#define FLAG_B 4
#define FLAG_M 5
#define FLAG_C 6
#define FLAG_ERROR 7
#define FLAG_E 8
#define FLAG_ACKE 9
#define FLAG_LREQUEST 10
#define FLAG_LCOUNT 11
#define FLAG_LRESPONSE 12
#define FLAG_LFINISH 13

#define FLAGOFFSET 2

#ifndef CLNT_H
#define CLNT_H

struct subclient{
    char handle[HANDLELENGTH+1] = {0};
    uint32_t socket;
};

#endif
