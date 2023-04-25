#include "serverObj.hpp"

void Server::serverAction(){
    int action;

    //poll all current sockets
    if((action = pollCall(-1)) != -1){

        if(action == serverSocket){
            addNewClient(action, serverSocket);
        }
        else{
            processPDU(action);
        }
    }
}

void Server::cascadeB(uint8_t senderHandle[HANDLELENGTH], uint8_t PDU[MAXBUFF]){

	Crowd clientList = clientTable.getClients();

    for(int client = 0; client < clientList.count; client++){
        if(clientList.clients[client].handle != senderHandle){
            //TODO remove length addition from sendPDU
            sendPDU(clientList.clients[client].socket, PDU);
        }
    }
}