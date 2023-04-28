#include "includes.hpp"
#include "crowd.hpp"

#ifndef TABLE_H
#define TABLE_H

#define HANDLELENGTH 100
#define DEFAULTSIZE 100
#define SIZETHRESH 0.5



class Clientele{

    private:
        subclient** clients;
        uint32_t size;
        uint32_t clientCount;

        uint32_t hash(const char* handle);
        void expandTable();
        void fillCheck();

    public:
        Clientele();
        bool insertClient(const char* handle, int socket);
        int getClientPort(char* handle);
        void removeClientHandle(char* handle);
        void removeClientSocket(int target);
        Crowd getClients();

        uint32_t getCount(){return clientCount;}


};


#endif