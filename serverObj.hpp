#include "includes.hpp"
#include "crowd.hpp"
#include "clientTable.hpp"
#include "safeUtil.h"
#include "networks.h"
#include "IOcontrol.hpp"
#include "pollLib.h"

#define FLAGCOUNT 11
#define HANDLELENGTH_POS 3
#define HANDLE_POS 4
#define LCOUNT_LENGTH 4
#define FLAGACTION uint8_t PDU[MAXBUF], uint32_t messageLength, uint32_t socket


#ifndef SERVER_H
#define SERVER_H

class Server{

    private:

    //list of all actions to take depending on recieved flag
    void (Server::*flagActions[FLAGCOUNT])(FLAGACTION) = {
        &Server::errorFlag, &Server::handshake, &Server::errorFlag, 
        &Server::errorFlag, &Server::cascadeB, &Server::forwardCM, 
        &Server::forwardCM, &Server::errorFlag, &Server::ackE, 
        &Server::errorFlag, &Server::respondL};

    Clientele clientTable;
    uint32_t serverSocket;

    //various actions to take depending on the flag of the incoming packet
    void ackE(FLAGACTION);
    void cascadeB(FLAGACTION);
    void forwardCM(FLAGACTION);
    void respondL(FLAGACTION);
    void handshake(FLAGACTION);
    void errorFlag(FLAGACTION);

    void parsePDU(uint8_t PDU[MAXBUF], uint32_t messageLength, uint32_t socket);
    
    void processPDU(uint32_t socket);
    void addNewClient(uint32_t socket);
    uint8_t readFlag(const uint8_t PDU[MAXBUF]);

    public:
    void serverAction();
    Server(uint32_t portnum);
};


#endif