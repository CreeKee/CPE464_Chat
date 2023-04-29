/******************************************************************************
* myClient.c
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
#include "client.hpp"

#define DEBUG_FLAG 1

void checkArgs(int argc, char * argv[]);

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], 0);

	//set up client
	Client cclient(socketNum, (uint8_t*)argv[1]);

	//run client in infinite loop. cclient will end the program when appropriate
	while(1){
		cclient.clientAction();
	}
	
	return 0;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s host-name port-number desired-handle\n", argv[0]);
		exit(1);
	}else if(strlen(argv[1]) > HANDLELENGTH-1){
		printf("Invalid handle, handle longer than 100 characters: %s\n", argv[1]);
		exit(-1);
	}
	else if(argv[1][0]<65 || argv[1][0] >122){
		printf("Invalid handle, handle starts with a number\n");
		exit(-1);
	}

}

