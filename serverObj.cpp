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
		parsePDU(dataBuffer, messageLen);
        
	}
	else
	{
		printf("Connection closed by other side\n");
        clientTable.removeClientSocket(socket);
		removeFromPollSet(socket);
	}

}

void Server::cascadeB(uint8_t PDU[MAXBUF], int messageLength){

    //TODO extract sender's handle

    int sent = 0;
	Crowd clientList = clientTable.getClients();

    for(int client = 0; client < clientList.count; client++){
        printf("FLAG sending to %s on socket %d\n",clientList.clients[client].handle, clientList.clients[client].socket);
        fflush(stdout);
        //if(strcmp(clientList.clients[client].handle,(const char*)senderHandle) != 0){
            //TODO handle return value
            sent = safeSend(clientList.clients[client].socket, PDU, messageLength, 0);
            if (sent < 0)
            {
                perror("send call");
                exit(-1);
            }

            printf("Amount of data sent is: %d\n", sent);
        //}
        printf("FLAG sent1\n");
        fflush(stdout);
    }
    printf("FLAG sent\n");
        fflush(stdout);
}

void Server::handshake(int clientSock, int flag){
    uint8_t dataBuffer[MAXBUF];
    
    sendPDU(clientSock, dataBuffer, flag);

}

void Server::parsePDU(uint8_t PDU[MAXBUF], int messageLength){
    printf("FLAG parsing\n");
    fflush(stdout);
    //TODO extract flag
    (this->*flagActions[0/*getFlag(PDU)*/])(PDU, messageLength);
}

void Server::errorFlag(uint8_t PDU[MAXBUF], int messageLength){
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

    printf("accepted new client\n");
    fflush(stdout);

    if ((len = recvPDU(socket, PDU, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}
    printf("recieved new client handshake\n");
    fflush(stdout);
	//check for disconnection
	if (len > 0)
	{
        if(readFlag(PDU) == FLAG_NEWCLIENT){
            if(clientTable.insertClient(handle, sock)){
                addToPollSet(sock);
                handshake(socket, FLAG_ACCEPTCLIENT);
            }
            else{
                //TODO
                handshake(socket, FLAG_REJECTCLIENT);
                printf("client tried to join with invalid handle");
            }
        }
        else{
            printf("client send invalid handshake\n");
            //TODO error check
            close(socket);
        }
	}
	else
	{
		printf("Connection closed by other side before sending 01 flag\n");
	}


    
	return;
}

uint8_t Server::readFlag(const uint8_t PDU[MAXBUF]){
    return PDU[FLAGOFFSET];
}