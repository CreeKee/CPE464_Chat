#include "includes.hpp"
#include "crowd.hpp"
#include "clientTable.hpp"

#ifndef SERVER_H
#define SERVER_H

#define HANDLELENGTH 100
#define DEFAULTSIZE 100
#define SIZETHRESH 0.5



class Server{
    private:
    Clientele clientTable;

    void serverAction();
    void cascadeB(uint8_t pdu[MAXBUF], Clientele ClientTable);

};

#endif