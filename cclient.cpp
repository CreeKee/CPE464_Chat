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

#define MAXBUF 1024
#define DEBUG_FLAG 1

void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void sendHandshake(int serverSocket, char* handle);

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	int action;
	uint8_t dataBuffer[MAXBUF];
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG);
	sendHandshake(socketNum, argv[3]);

	//create poll
	setupPollSet();
	//add server socket to poll set to listen for new connections
	addToPollSet(socketNum);
	addToPollSet(STDIN_FILENO);

	sendToServer(socketNum);
	while(1){
		//TODO
		if((action = pollCall(-1)) != -1){

			switch(action){
				case STDIN_FILENO:
					sendToServer(socketNum);
					break;
				default:
					recvPDU(action, dataBuffer, MAXBUF);
					printf("->%s\n", dataBuffer+2);
					break;
			}
    	}
	}
	
	close(socketNum);
	
	return 0;
}

void sendToServer(int socketNum)
{
	uint8_t sendBuf[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
	
	sendLen = readFromStdin(sendBuf);
	printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);
	
	prependLength(sendBuf,sendLen);
	sent = safeSend(socketNum, sendBuf, sendLen+LENGTHFIELD, 0) - LENGTHFIELD;
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Amount of data sent is: %d\n", sent);
}

int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	printf("Enter data: ");
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s host-name port-number desired-handle\n", argv[0]);
		exit(1);
	}
}

void sendHandshake(int serverSocket, char* handle){

	uint8_t buffer[MAXBUF];
	uint32_t hLen = strlen(handle);

	if(hLen > HANDLELENGTH){
		//TODO magic num
		if(handle[0]<65 || handle[0] >122){
			printf("handle must start with an alphabetical character\n");
			exit(-1);
		}
		else{
			buffer[0] = hLen;
			memcpy(buffer+1, handle, hLen+1);
			sendPDU(serverSocket, buffer, FLAG_NEWCLIENT);

		}
	}
	else{
		printf("handle too long\n");
		exit(-1);
	}

}