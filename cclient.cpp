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
#include <algorithm>
#include "client.hpp"

#define MAXBUF 1024
#define DEBUG_FLAG 1



void checkArgs(int argc, char * argv[]);

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG);

	Client cclient(socketNum, (uint8_t*)argv[3]);

	while(1){

		cclient.clientAction();


		//TODO
	}
	
	close(socketNum);
	
	return 0;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s host-name port-number desired-handle\n", argv[0]);
		exit(1);
	}else if(strlen(argv[3]) > HANDLELENGTH-1){
		printf("handle [%s] too long\n",argv[4]);
		exit(-1);
	}
	else if(argv[3][0]<65 || argv[3][0] >122){
		printf("handle must start with an alphabetical character\n");
		exit(-1);
	}
}

