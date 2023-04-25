#include "includes.hpp"
#include "serverOBJ.hpp"
#include "safeUtil.h"
#include "networks.h"
#include "IOcontrol.hpp"
#include "pollLib.h"
#include "clientTable.hpp"



int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	Clientele clientTable;
	
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	Server mainServer(portNumber);

	// wait for client to connect
	//clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
	while(1){
		mainServer.serverAction();
	}
	
	/* close the sockets */
	close(clientSocket);
	close(mainServerSocket);

	
	return 0;
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
