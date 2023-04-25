#include "includes.hpp"
#include "serverOBJ.hpp"

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	Clientele clientTable;
	
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);

	// wait for client to connect
	//clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);

	initializeServer(mainServerSocket);
	runServer();
	
	/* close the sockets */
	close(clientSocket);
	close(mainServerSocket);

	
	return 0;
}

void initializeServer(int serverSocket){

	//create poll
	setupPollSet();
	
	//add server socket to poll set to listen for new connections
	addToPollSet(serverSocket);
}

void runServer(){
	
	//TODO

}

void processPDU(int socket){

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
		//parsePDU();
		cascadeB(dataBuffer);
	}
	else
	{
		printf("Connection closed by other side\n");
		removeFromPollSet(socket);
	}
	return;
}

