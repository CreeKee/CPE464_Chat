#include "includes.hpp"
#include "crowd.hpp"
#include "clientTable.hpp"
#include "safeUtil.h"
#include "networks.h"
#include "IOcontrol.hpp"
#include "pollLib.h"

#define FLAGCOUNT 13
#define FLAGOFFSET 2


#ifndef SERVER_H
#define SERVER_H



class Server{

    private:
    void (Server::*flagActions[FLAGCOUNT])(uint8_t[], int ) = {
        &Server::cascadeB, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag, &Server::errorFlag};
    Clientele clientTable;
    int serverSocket;

    void cascadeB(uint8_t PDU[MAXBUF], int messageLength);
    void handshake(int clientSock, int flag);

    void parsePDU(uint8_t PDU[MAXBUF], int messageLength);
    void errorFlag(uint8_t PDU[MAXBUF], int messageLength);
    void processPDU(int socket);
    void addNewClient(int socket, char* handle);
    uint8_t readFlag(const uint8_t PDU[MAXBUF]);

    public:
    void serverAction();
    Server(int portnum);
};


#endif