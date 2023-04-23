/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include "includes.hpp"
#include "safeUtil.h"
#include "networks.h"
#include "IOcontrol.hpp"
#include "pollLib.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);
void serverControl(int serverSocket);
void addNewSocket(int newClient, int mainServerSocket);
void processClient(int clientSocket);

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);

	// wait for client to connect
	//clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);

	serverControl(mainServerSocket);
	
	/* close the sockets */
	close(clientSocket);
	close(mainServerSocket);

	
	return 0;
}

void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	if (messageLen > 0)
	{
		printf("Message received, length: %d Data: %s\n", messageLen, dataBuffer);
	}
	else
	{
		printf("Connection closed by other side\n");
	}
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

void serverControl(int serverSocket){
	int action;

	//create poll
	setupPollSet();
	
	//add server socket to poll set to listen for new connections
	addToPollSet(serverSocket);

	//infinite loop as per lab instructions
	while(1){

		//poll all current sockets
		if((action = pollCall(0)) != -1){

			if(action == serverSocket){
				addNewSocket(action, serverSocket);
			}
			else{
				processClient(action);
			}

		}
			
	}
	return;
}

void addNewSocket(int newClient, int mainServerSocket){
	
	int sock;

	//add new socket to the poll
	if((sock = tcpAccept(mainServerSocket, DEBUG_FLAG)) == -1){
		perror("failed to accept new client");

		//current behavior for failed accept it to exit.
		exit(1);
	}

	addToPollSet(sock);
	return;
}

void processClient(int clientSocket){

	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//get data from clientSocket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	//check for disconnection
	if (messageLen > 0)
	{
		printf("Message received on socket: %d, length: %d Data: %s\n", clientSocket, messageLen, dataBuffer);
	}
	else
	{
		printf("Connection closed by other side\n");
		removeFromPollSet(clientSocket);
	}
	return;
}