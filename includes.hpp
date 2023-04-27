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

#define MAXMSG 200

#define MAXBUF 1024
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


#ifndef CLNT_H
#define CLNT_H
struct client{
    char handle[HANDLELENGTH];
    int socket;
};
#endif