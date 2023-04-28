#include "clientTable.hpp"

Clientele::Clientele(){
    clients = new subclient*[DEFAULTSIZE];
    size = DEFAULTSIZE;
    clientCount = 0;
    for(uint32_t init = 0; init < size; init++){
        clients[init] = NULL;
    }
}

/*
insertClient adds a new client to the table
*/
bool Clientele::insertClient(const char* handle, uint32_t socket){

    uint32_t hashVal = hash(handle);
    bool valid = true;
    
    printf("inserting [%s] with length %d", handle, strlen(handle));
    fflush(stdout);

    //scan to next available bucket and check for duplicates
    while(clients[hashVal] != NULL && valid == true){
        valid = (strcmp(clients[hashVal]->handle,handle) != 0);
        hashVal = (hashVal+1)%size;
    }
    if(valid){

        //create new client entry in the table
        if((valid = strlen(handle)<HANDLELENGTH)){

            //create and initialize subclient
            clients[hashVal] = new subclient;
            memcpy(clients[hashVal]->handle, handle, strlen(handle)+1);
            clients[hashVal]->socket = socket;
            clientCount++;

            //check for redistribuiton
            fillCheck();
        }
    }
    return valid;
}

/*
getClientPort takes a client's handle and returns their port number
*/
int32_t Clientele::getClientPort(char* handle){

    uint32_t hashVal = hash(handle);

    //find client's entry
    while(clients[hashVal] != NULL && strcmp(clients[hashVal]->handle,handle) != 0){
        hashVal = (hashVal+1)%size;
    }

    //return client's port number
    return clients[hashVal] != NULL ? clients[hashVal]->socket:-1;
}

/*
removeClientHanlde takes a client's handle, finds their entry, and removes them
*/
void Clientele::removeClientHandle(char* handle){

    int hashVal = hash(handle);

    //locate client
    while(clients[hashVal] != NULL && strcmp(clients[hashVal]->handle,handle) != 0){
        hashVal = (hashVal+1)%size;
    }

    //remove client
    if(clients[hashVal] != NULL){
        free(clients[hashVal]->handle);
        free(clients[hashVal]);
        clients[hashVal] = NULL;
        clientCount--;
    }
    else{
        perror("could not free socket with given handle");
    }
}

/*
removeClientSocket scans the entire table to find the client with the desired port number
and remove their entry
*/
void Clientele::removeClientSocket(uint32_t target){

    bool found = false;

    //locate target
    for(uint32_t index = 0; index < size && found == false; index++){

        //when found remove the client
        if(clients[index] != NULL && clients[index]->socket == target){
            found = true;
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


/*
getClients coalates and returns a crowd containing all of the current clients
*/
Crowd Clientele::getClients(){

    Crowd retval(clientCount);

    //collect all clients
    for(uint32_t index = 0, found = 0; index < size && found < clientCount; index++){
        printf("<>%d\n", index);
        fflush(stdout);
        if(clients[index] != NULL){
            printf("found client at %d\n", index);
            fflush(stdout);
            //add client to crowd
            retval.clients[found].socket = clients[index]->socket;
            memcpy(retval.clients[found].handle, clients[index]->handle, HANDLELENGTH);
            found++;
        }
    }
    return retval;
}

/*
fillCheck checks if the client table is too full and needs to expand
*/
void Clientele::fillCheck(){
    if(((double)clientCount/size) > SIZETHRESH){
        expandTable();
    }
}

/*
hash uses the djb2 algorithm to determine the index for a client's handle
*/
uint32_t Clientele::hash(const char* handle){
    uint32_t hash = 5381;
    uint16_t c;

    while ((c = *handle++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash%size;
}

/*
expandTable doubles the table size and reinserts all clients
*/
void Clientele::expandTable(){

    //save old table
    subclient** oldTable = clients;
    uint32_t oldSize = size;

    //create new table, and double table size
    size = size<<1;
    clients = new subclient*[size];

    //initialize new table to NULL
    for(uint32_t index = 0; index < size; index++){
        clients[index] = NULL;
    }

    //reinsert clients
    for(int index = 0; index < oldSize; index++){

        if(oldTable[index] != NULL){
            insertClient(oldTable[index]->handle, oldTable[index]->socket);
            free(oldTable[index]);
        }
    }

    //free old client table
    free(oldTable);
    return;
}

Clientele::~Clientele(){

    for(int index = 0; index < size; index++){
        if(clients[index] != NULL){
            free(clients[index]);
        }
    }
    free(clients);

}