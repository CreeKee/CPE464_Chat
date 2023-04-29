#include "includes.hpp"
#include "crowd.hpp"

#define HANDLELENGTH 100
#define DEFAULTSIZE 100
#define SIZETHRESH 0.5

#ifndef TABLE_H
#define TABLE_H


class Clientele{

    private:
        subclient** clients;
        int size;
        int clientCount;

        uint32_t hash(const char* handle);
        void expandTable();
        void fillCheck();

    public:
        Clientele();
        bool insertClient(const char* handle, int socket);
        int getClientPort(char* handle);
        void removeClientHandle(char* handle);
        void removeClientSocket(uint32_t target);
        Crowd getClients();


};


#endif