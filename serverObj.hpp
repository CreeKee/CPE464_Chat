#include "includes.hpp"
#include "crowd.hpp"
#include "clientTable.hpp"
#include "safeUtil.h"
#include "networks.h"
#include "IOcontrol.hpp"
#include "pollLib.h"

#define FLAGCOUNT 13
#define FLAGOFFSET 2
#define HANDLELENGTH_POS 3
#define HANDLE_POS 4
#define CM_COUNT_POS
#define FLAGACTION uint8_t PDU[MAXBUF], int messageLength, int socket


#ifndef SERVER_H
#define SERVER_H

class Server{

    private:
    void (Server::*flagActions[FLAGCOUNT])(FLAGACTION) = {
        &Server::errorFlag, &Server::handshake, &Server::errorFlag, 
        &Server::errorFlag, &Server::cascadeB, &Server::forwardCM, 
        &Server::forwardCM, &Server::errorFlag, &Server::errorFlag, 
        &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, 
        &Server::errorFlag};

    Clientele clientTable;
    int serverSocket;

    void cascadeB(FLAGACTION);
    void forwardM(FLAGACTION);
    void forwardCM(FLAGACTION);
    void handshake(FLAGACTION);
    void errorFlag(FLAGACTION);

    void parsePDU(uint8_t PDU[MAXBUF], int messageLength, int socket);
    
    void processPDU(int socket);
    void addNewClient(int socket, char* handle);
    uint8_t readFlag(const uint8_t PDU[MAXBUF]);

    public:
    void serverAction();
    Server(int portnum);
};


#endif