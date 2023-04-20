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
void addNewSocket(int newClient);
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
	setupPollSet();
	addToPollSet(serverSocket);
	printf("flag0\n");
	while(1){

		if((action = pollCall(1000)) != -1){
			printf("flag1\n");
			if(action == serverSocket){
				printf("flag2\n");
				addNewSocket(action);
			}
			else{
				printf("flag3\n");
				processClient(action);
			}

		}
		else{
			printf("poll failed %d\n", action);
		}
			
	}
	return;
}

void addNewSocket(int newClient, int mainServerSocket){
	
	addToPollSet(tcpAccept(mainServerSocket, DEBUG_FLAG));
	return;
}

void processClient(int clientSocket){

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
		printf("Message received on socket: %d, length: %d Data: %s\n", clientSocket, messageLen, dataBuffer);
	}
	else
	{
		printf("Connection closed by other side\n");
	}
	return;
}