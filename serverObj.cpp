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
    printf("FLAG cascading\n");
    fflush(stdout);

	Crowd clientList = clientTable.getClients();

    for(int client = 0; client < clientList.count; client++){
        //if(strcmp(clientList.clients[client].handle,(const char*)senderHandle) != 0){
            //TODO handle return value
            safeSend(clientList.clients[client].socket, PDU, messageLength, 0);
        //}
    }
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

	//add new socket to the poll
	if((sock = tcpAccept(serverSocket, DEBUG_FLAG)) == -1){
		perror("failed to accept new client");

		//current behavior for failed accept it to exit.
		exit(1);
	}
    if(clientTable.insertClient(handle, socket)){
	    addToPollSet(sock);
    }
    else{
        //TODO
        perror("client tried to join with invalid handle");
    }
    
	return;
}