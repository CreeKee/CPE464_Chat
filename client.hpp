#include "includes.hpp"

#include "safeUtil.h"
#include "networks.h"
#include "IOcontrol.hpp"
#include "pollLib.h"
#include <algorithm>

#define PRINTACTIONS 14
#define RECVACTION uint8_t PDU[MAXBUF]

#ifndef CLIENT_H
#define CLIENT_H

class Client{

    private:

    //list of all actions to take depending on recieved flag
    void (Client::*flagActions[PRINTACTIONS])(RECVACTION) = {
        &Client::errorFlag, &Client::errorFlag, &Client::ignoreFlag, 
        &Client::badConnect, &Client::displayB, &Client::displayCM, 
        &Client::displayCM, &Client::badHandle, &Client::errorFlag, 
        &Client::consign, &Client::errorFlag, &Client::prepL,
        &Client::displayL, &Client::finishL};

    bool print$ = true;
    int serverSock;
    uint8_t myhLen;
    uint8_t handle[HANDLELENGTH];

    void compileE();
    void compileL();
    void compileCM(uint8_t buffer[MAXBUF], int buflen, uint8_t dstCount, int flag);
    void compileB(uint8_t buffer[MAXBUF], int buflen);

    void fragment(uint8_t PDU[MAXBUF], uint8_t* buffer, int buflen, int dataStart, int flag);
    void createMessage();
    uint32_t readFromStdin(uint8_t * buffer);
    void recvFromServer(int serverSock);
    uint8_t displayHandle(uint8_t PDU[MAXBUF]);
    void sendHandshake();
    int32_t getcomp(uint8_t** s);

    //various actions to take depending on the flag of the incoming packet
    void errorFlag(RECVACTION){printf("\nERROR: recieved unkown flag\n");}
    void ignoreFlag(RECVACTION){return;};
    void badConnect(RECVACTION){printf("Handle already in use: %s\n",handle); exit(1);}
    void displayB(RECVACTION){printf("%s\n", PDU+displayHandle(PDU)+1);}
    void displayCM(RECVACTION);
    void badHandle(RECVACTION){printf("\nClient with handle %.*s does not exist\n", PDU[0], PDU+1);}
    void consign(RECVACTION){close(serverSock); exit(0);}
    void prepL(RECVACTION){printf("\nNumber of clients: %d\n", ntohl(*(uint32_t*)PDU)); print$ = false;}
    void displayL(RECVACTION){printf("\t%.*s\n",PDU[0], PDU+1);}
    void finishL(RECVACTION){print$=true;}

    

    public:
    Client(uint32_t socket, uint8_t myhandle[HANDLELENGTH]);
    void clientAction();

};

#endif