#include "serverObj.hpp"


Server::Server(int portnum){

    clientTable = Clientele();
    serverSocket = tcpServerSetup(portnum);

    //create poll
	setupPollSet();
	
	//add server socket to poll set to listen for new connections
	addToPollSet(serverSocket);
}

void Server::serverAction(){
    int action;

    //poll all current sockets
    if((action = pollCall(-1)) != -1){

        if(action == serverSocket){
            addNewClient(serverSocket, "test handle");
        }
        else{
            processPDU(action);
        }
    }
}

void Server::processPDU(int socket){

	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//get data from clientSocket
	if ((messageLen = recvPDU(socket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	//check for disconnection
	if (messageLen > 0)
	{
        printf("socket: %d\n", socket);
		parsePDU(dataBuffer, messageLen, socket);
        
	}
	else
	{
		printf("Connection closed by other side\n");
        clientTable.removeClientSocket(socket);
		removeFromPollSet(socket);
	}

}

void Server::cascadeB(FLAGACTION){

	Crowd clientList = clientTable.getClients();

    for(int client = 0; client < clientList.count; client++){
        printf("FLAG sending to %s on socket %d\n",clientList.clients[client].handle, clientList.clients[client].socket);
        fflush(stdout);
        if(clientList.clients[client].socket != socket){

            forwardPDU(clientList.clients[client].socket, PDU, messageLength);
        }

    }

}

void Server::forwardM(FLAGACTION){
    uint8_t targetHandle[HANDLELENGTH+1] = {0};
    int destPort;
    
    //TODO magic nums
    memcpy(targetHandle, PDU+HANDLE_POS+PDU[HANDLELENGTH_POS]+2, PDU[HANDLELENGTH_POS+PDU[HANDLELENGTH_POS]+2]);

    //TODO confirmations
    if((destPort = clientTable.getClientPort((char*)targetHandle))!=-1){
        forwardPDU(destPort, PDU, messageLength);
    }
    else{
        
        printf("%M to invalid client [%s]\n", targetHandle);
    }

    
}

void Server::handshake(FLAGACTION){
    uint8_t handle[HANDLELENGTH];
    uint8_t buffer[MAXBUF];

    memcpy(handle, PDU+HANDLE_POS, PDU[HANDLELENGTH_POS]);
    printf("attempting to add new client with handle [%s]\n", handle);
    if(clientTable.insertClient((char*)handle, socket)){
        sendPDU(socket, buffer,0, FLAG_ACCEPTCLIENT);
    }
    else{
        //TODO
        sendPDU(socket, buffer,0, FLAG_REJECTCLIENT);
        printf("client tried to join with invalid handle");
    }

}

void Server::parsePDU(uint8_t PDU[MAXBUF], int messageLength, int socket){
    printf("FLAG parsing\n");
    fflush(stdout);
    //TODO extract flag
    (this->*flagActions[readFlag(PDU)])(PDU, messageLength, socket);
}

void Server::errorFlag(FLAGACTION){
    perror("server recieved PDU with an invalid flag\n");
    exit(-1);
}

void Server::addNewClient(int socket, char* handle){
	
	int sock;
    int len;
    uint8_t PDU[MAXBUF];

	//add new socket to the poll
	if((sock = tcpAccept(serverSocket, DEBUG_FLAG)) == -1){
		perror("failed to accept new client");

		//current behavior for failed accept it to exit.
		exit(1);
	}

    addToPollSet(sock);



    
	return;
}

uint8_t Server::readFlag(const uint8_t PDU[MAXBUF]){
    return PDU[FLAGOFFSET];
}



