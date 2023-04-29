#include "clientTable.hpp"



Clientele::Clientele(){
    clients = new subclient*[DEFAULTSIZE];
    size =DEFAULTSIZE;
    clientCount = 0;
    for(int init = 0; init < size; init++){
        clients[init] = NULL;
    }
}

bool Clientele::insertClient(const char* handle, int socket){

    int hashVal = hash(handle);
    bool valid = true;

    while(clients[hashVal] != NULL && valid == true){
        valid = (strcmp(clients[hashVal]->handle,handle) != 0);
        hashVal = (hashVal+1)%size;
    }
    if(valid){

        if((valid = strlen(handle)<HANDLELENGTH)){
            clients[hashVal] = new subclient;
            memcpy(clients[hashVal]->handle, handle, strlen(handle)+1);
            clients[hashVal]->socket = socket;
            clientCount++;
            fillCheck();
        }
    }
    return valid;
}

int Clientele::getClientPort(char* handle){

    int hashVal = hash(handle);
    while(clients[hashVal] != NULL && strcmp(clients[hashVal]->handle,handle) != 0){
        hashVal = (hashVal+1)%size;
    }

    return clients[hashVal] != NULL ? clients[hashVal]->socket:-1;
}

void Clientele::removeClientHandle(char* handle){
    int hashVal = hash(handle);
    while(clients[hashVal] != NULL && strcmp(clients[hashVal]->handle,handle) != 0){
        hashVal = (hashVal+1)%size;
    }

    if(clients[hashVal] != NULL){
        //TODO close socket
        free(clients[hashVal]->handle);
        free(clients[hashVal]);
        clients[hashVal] = NULL;
        clientCount--;
    }
    else{
        perror("could not free socket with given handle");
    }
}

void Clientele::removeClientSocket(uint32_t target){
    bool found = false;
    for(int index = 0; index < size && found == false; index++){
        if(clients[index] != NULL && clients[index]->socket == target){
            found = true;
            //TODO close socket
            free(clients[index]);
            clients[index] = NULL;
            clientCount--;
        }
    }
    if(found == false){
        perror("Could not find given socket to remove");
    }
    return;
}

Crowd Clientele::getClients(){

    Crowd retval(clientCount);

    for(int index = 0, found = 0; index < size && found < clientCount; index++){
        if(clients[index] != NULL){
            retval.clients[found].socket = clients[index]->socket;
            memcpy(retval.clients[found].handle, clients[index]->handle, HANDLELENGTH);
            found++;
        }
    }
    return retval;
}

void Clientele::fillCheck(){
    if(((double)clientCount/size) > SIZETHRESH){
        expandTable();
    }
}

uint32_t Clientele::hash(const char* handle){
    uint32_t hash = 5381;
    uint16_t c;

    while ((c = *handle++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash%size;
}

void Clientele::expandTable(){

    printf("expanding table\n");

    subclient** oldTable = clients;
    int oldSize = size;
    clientCount = 0;
    size = size<<1;
    clients = new subclient*[size];


    for(int index = 0; index < size; index++){
        clients[index] = NULL;
    }

    for(int index = 0; index < oldSize; index++){
        if(oldTable[index] != NULL){
            insertClient(oldTable[index]->handle, oldTable[index]->socket);
            free(oldTable[index]);
        }
    }
    free(oldTable);
    return;
}