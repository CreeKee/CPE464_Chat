#include "includes.hpp"

#ifndef CROWD_H
#define CROWD_H

class Crowd{

    public:
        subclient* clients;
        uint32_t count;

        Crowd(uint32_t clientCount){
            count = clientCount;
            clients = new subclient[count];
        };

        ~Crowd(){
            free(clients);
        }

};

#endif