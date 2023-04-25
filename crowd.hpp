#include "includes.hpp"

#ifndef CROWD_H
#define CROWD_H

class Crowd{

    public:
        client* clients;
        int count;

        Crowd(int clientCount){
            count = clientCount;
            clients = new client[count];
        };

        ~Crowd(){
            free(clients);
        }

};

#endif