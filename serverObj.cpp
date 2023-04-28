#include "serverObj.hpp"

Server::Server(uint32_t portnum){

    clientTable = Clientele();
    serverSocket = tcpServerSetup(portnum);

    //create poll
	setupPollSet();
	
	//add server socket to poll set to listen for new connections
	addToPollSet(serverSocket);
}

/*
serverAction polls available sockets, and determines what action
to take when one become ready. Either recieve and process a packet
from a client, or add a new client
*/
void Server::serverAction(){

    uint32_t action;

    //poll all current sockets
    if((action = pollCall(-1)) != -1){

        if(action == serverSocket){
            addNewClient(serverSocket);
        }
        else{
            processPDU(action);
        }
    }
}

/*
processPDU recieves a packet from a client and chooses the next action
*/
void Server::processPDU(uint32_t socket){

	uint8_t dataBuffer[MAXBUF];
	uint32_t messageLen = 0;
	
	//get data from clientSocket
	if ((messageLen = recvPDU(socket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	//check for disconnection
	if (messageLen > 0)
	{
        //handle packet form existing client
		parsePDU(dataBuffer, messageLen, socket);
	}
	else
	{
        //client has been abruptly terminated, remove them
        clientTable.removeClientSocket(socket);
		removeFromPollSet(socket);
	}

}

/*
ackE responds to a %E message and removes the client from further actions
*/
void Server::ackE(FLAGACTION){

    uint8_t buffer[MAXBUF] = {0};

    //remove client from communications from the server
    removeFromPollSet(socket);
    clientTable.removeClientSocket(socket);

    //send ACK message
    sendPDU(socket, buffer, 0, FLAG_ACKE);

    //do not close the socket, let the client recieve and close on their end
}

/*
cascadeB recieves a %B message, then forwards it to all
clients EXCEPT the original sender
*/
void Server::cascadeB(FLAGACTION){

    //get list of all clients
	Crowd clientList = clientTable.getClients();

    //iterate through all clients
    for(uint32_t client = 0; client < clientList.count; client++){

        //make sure not to send back to the original sender
        if(clientList.clients[client].socket != socket){

            //forward the %B message
            forwardPDU(clientList.clients[client].socket, PDU, messageLength);
        }
    }
}

/*
forwardCM takes a %C or %M message and forwards it to all listed targets
*/
void Server::forwardCM(FLAGACTION){
    
    uint8_t errorbuf[MAXBUF] = {0};
    uint8_t targetHandle[HANDLELENGTH+1] = {0};
    uint32_t destPort;
    uint8_t curLen;
    uint8_t* curDst = PDU+HANDLE_POS+PDU[HANDLELENGTH_POS]+2;

    //extract all targets
    for(uint32_t cnt = 0; cnt<PDU[HANDLE_POS+PDU[HANDLELENGTH_POS]]; cnt++){
        //extract current target's handle
        curLen = curDst[-1];
        memcpy(targetHandle, curDst, curLen);
        curDst+=curLen+1;

        //send to current handle if they exist
        if((destPort = clientTable.getClientPort((char*)targetHandle))!=-1){
            forwardPDU(destPort, PDU, messageLength);
        }
        else{
            insertHandle(errorbuf, targetHandle, curLen);
            sendPDU(socket, errorbuf, curLen+1, FLAG_ERROR);
        }
    }
}

/*
respondL responds to a %L message by sending appropriate flag 11, 12, and 13 messages
*/
void Server::respondL(FLAGACTION){

    uint8_t buffer[MAXBUF] = {0};

    //get list of all clients
    Crowd clients = clientTable.getClients();

    //convert client count to network order and send flag 11 message
    //((uint32_t*) buffer)[0] = htonl(clients.count);

    //sendPDU(socket, buffer, LCOUNT_LENGTH, FLAG_LCOUNT);

    //send flag 12 message for each client
    //for(uint32_t clnt = 0; clnt < clients.count; clnt++){
    //    insertHandle(buffer, (uint8_t*)clients.clients[clnt].handle, strlen(clients.clients[clnt].handle));
        //sendPDU(socket, buffer,strlen(clients.clients[clnt].handle)+1,FLAG_LRESPONSE);
   // }

    //send final flag 13 message
    //sendPDU(socket, buffer, 0, FLAG_LFINISH);

}

/*
handshake confirms connection of a new client and checks validity of their handle
*/
void Server::handshake(FLAGACTION){

    uint8_t handle[HANDLELENGTH];
    uint8_t buffer[MAXBUF];

    //extract desired handle
    memcpy(handle, PDU+HANDLE_POS, PDU[HANDLELENGTH_POS]);

    //check availability of handle
    printf("adding clinet [%s]\n",(char*)handle);
    if(clientTable.insertClient((char*)handle, socket)){
        sendPDU(socket, buffer,0, FLAG_ACCEPTCLIENT);
    }
    else{
        sendPDU(socket, buffer,0, FLAG_REJECTCLIENT);
        removeFromPollSet(socket);
        close(socket);
    }
}

/*
parsPDU extracts the flag from a packet, and chooses the next action
*/
void Server::parsePDU(uint8_t PDU[MAXBUF], uint32_t messageLength, uint32_t socket){

    //extract flag
    uint8_t flag = readFlag(PDU);

    //choose next action
    if( flag<= FLAGCOUNT && flag >=0){
        (this->*flagActions[flag])(PDU, messageLength, socket);
    }
    else{
        printf("ERROR: bad flag read\n");
    }
}

/*
errorFlag handles any cases where an invalid flag is sent to the sever
*/
void Server::errorFlag(FLAGACTION){
    printf("ERROR: server recieved PDU with an invalid flag\n");
}

/*
addNewClient accepts the connection to a new client and adds their socket to the 
pollset so their handshake can be recieved
*/
void Server::addNewClient(uint32_t socket){
	
	uint32_t sock;

	//add new socket to the poll
	if((sock = tcpAccept(serverSocket, DEBUG_FLAG)) == -1){
		perror("failed to accept new client");

        //do not exit, it was probably the client's fault anyways
	}
    else{
        addToPollSet(sock);
    }

	return;
}

uint8_t Server::readFlag(const uint8_t PDU[MAXBUF]){
    return PDU[FLAGOFFSET];
}



