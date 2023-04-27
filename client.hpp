#include "includes.hpp"

#include "safeUtil.h"
#include "networks.h"
#include "IOcontrol.hpp"
#include "pollLib.h"
#include <algorithm>

#ifndef CLIENT_H
#define CLIENT_H

class Client{

    private:
    int serverSock;
    uint8_t myhLen;
    uint8_t handle[HANDLELENGTH];

    void compileCM(uint8_t buffer[MAXBUF], int buflen, uint8_t dstCount, int flag);
    void fragment(uint8_t PDU[MAXBUF], uint8_t buffer[MAXBUF], int buflen, int dataStart, int flag);
    void createMessage();
    void sendToServer(int socketNum);
    int readFromStdin(uint8_t * buffer);
    void recvFromServer(int serverSock);
    int appendHandle(uint8_t* PDU, uint8_t* buffer);
    void insertHandle(uint8_t* PDUstart, uint8_t* handleStart, uint8_t hLen);
    void compileB(uint8_t buffer[MAXBUF], int buflen);
    
    void sendHandshake();

    public:
    Client(int socket, uint8_t myhandle[HANDLELENGTH]);
    void clientAction();

};

#endif