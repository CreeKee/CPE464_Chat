#include "includes.hpp"
#include "serverObj.hpp"
#include "safeUtil.h"
#include "networks.h"
#include "IOcontrol.hpp"
#include "pollLib.h"
#include "clientTable.hpp"


int checkArgs(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int portNumber = 0;
	Clientele clientTable;
	
	//get port number and verify arguments;
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	Server mainServer(portNumber);

	//run server
	while(1){
		mainServer.serverAction();
	}
	
	/* close the server socket */
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
