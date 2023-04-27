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

    void compileL();
    void compileCM(uint8_t buffer[MAXBUF], int buflen, uint8_t dstCount, int flag);
    void compileB(uint8_t buffer[MAXBUF], int buflen);

    void fragment(uint8_t PDU[MAXBUF], uint8_t* buffer, int buflen, int dataStart, int flag);
    void createMessage();
    void sendToServer(int socketNum);
    int readFromStdin(uint8_t * buffer);
    void recvFromServer(int serverSock);

    
    
    void sendHandshake();

    public:
    Client(int socket, uint8_t myhandle[HANDLELENGTH]);
    void clientAction();

};

#endif