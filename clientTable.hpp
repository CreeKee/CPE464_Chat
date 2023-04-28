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
        uint32_t size;
        uint32_t clientCount;

        uint32_t hash(const char* handle);
        void expandTable();
        void fillCheck();

    public:
        Clientele();
        ~Clientele();
        bool insertClient(const char* handle, uint32_t socket);
        int32_t getClientPort(char* handle);
        void removeClientHandle(char* handle);
        void removeClientSocket(uint32_t target);
        Crowd getClients();


};


#endif