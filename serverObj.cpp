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

void Server::ackE(FLAGACTION){
    uint8_t buffer[MAXBUF] = {0};
    removeFromPollSet(socket);
    clientTable.removeClientSocket(socket);
    sendPDU(socket, buffer, 0, FLAG_ACKE);
    //TODO close on client side
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

void Server::forwardCM(FLAGACTION){
    
    uint8_t errorbuf[MAXBUF] = {0};
    uint8_t targetHandle[HANDLELENGTH+1] = {0};
    int destPort;
    uint8_t curLen;
    uint8_t* curDst = PDU+HANDLE_POS+PDU[HANDLELENGTH_POS]+2;

    //TODO magic nums
    printf("client count = %d\n", HANDLE_POS+PDU[HANDLELENGTH_POS]);
    for(int cnt = 0; cnt<HANDLE_POS+PDU[HANDLELENGTH_POS]; cnt++){
        curLen = curDst[-1];
        memcpy(targetHandle, curDst, curLen);
        curDst+=curLen+1;

        //TODO confirmations
        if((destPort = clientTable.getClientPort((char*)targetHandle))!=-1){
            forwardPDU(destPort, PDU, messageLength);
        }
        else{
            insertHandle(errorbuf, targetHandle, curLen);
            sendPDU(socket, errorbuf, curLen+1, FLAG_ERROR);
            printf("M or C to invalid client [%s]\n", targetHandle);
        }
    }

}

void Server::respondL(FLAGACTION){

    uint8_t buffer[MAXBUF];
    Crowd clients = clientTable.getClients();

    ((uint32_t*) buffer)[0] = htonl(clients.count);
    sendPDU(socket, buffer, LCOUNT_LENGTH, FLAG_LCOUNT);
    printf("confirming count %d = %d", ((uint32_t*) buffer)[0], htonl(clients.count));

    for(int clnt = 0; clnt < clients.count; clnt++){
        insertHandle(buffer, (uint8_t*)clients.clients[clnt].handle, strlen(clients.clients[clnt].handle));
        sendPDU(socket, buffer,strlen(clients.clients[clnt].handle)+1,FLAG_LRESPONSE);
    }

    sendPDU(socket, buffer, 0, FLAG_LFINISH);

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
    int flag = readFlag(PDU);
    if( flag<= FLAGCOUNT && flag >=0){
        (this->*flagActions[flag])(PDU, messageLength, socket);
    }
    else{
        printf("bad flag read\n");
    }
}

void Server::errorFlag(FLAGACTION){
    perror("server recieved PDU with an invalid flag\n");
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



