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

#define MAXBUF 1024
#define DEBUG_FLAG 1

void compileCM(uint8_t buffer[MAXBUF], int buflen, int socket);
void createMessage(int serverSocket);
void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void recvFromServer(int serverSock);
void checkArgs(int argc, char * argv[]);
void sendHandshake(int serverSocket, char* handle);

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	int action;
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG);
	sendHandshake(socketNum, argv[3]);

	//create poll
	setupPollSet();
	//add server socket to poll set to listen for new connections
	addToPollSet(socketNum);
	addToPollSet(STDIN_FILENO);

	while(1){
		//TODO
		if((action = pollCall(-1)) != -1){

			switch(action){
				case STDIN_FILENO:
					createMessage(socketNum);
					break;
				default:
					recvFromServer(action);
					
					
					break;
			}
    	}
	}
	
	close(socketNum);
	
	return 0;
}


void createMessage(int serverSocket){
	uint8_t buffer[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0; 
	sendLen = readFromStdin(buffer);

	if(buffer[0] != '%'){
		printf("all commands must start with '%'\n");
	} else if(buffer[2] != ' '){
		printf("invalid command format");
	}
	else switch(tolower(buffer[1])){
		case 'm':
			compileCM(buffer+3, sendLen, serverSocket);
			break;
		default:
			printf("unkown command %c%c\n",buffer[0], buffer[1]);
			break;
	}
}

void compileCM(uint8_t buffer[MAXBUF], int buflen, int socket){
	uint8_t PDU[MAXBUF] = {0};
	int dataStart = 0;
	int currlen;
	uint8_t* splitter = (uint8_t*)strchr((char*)buffer, ' ');

	printf("got splitter: %s\n", buffer);
	fflush(stdout);

	if(splitter != NULL){

		dataStart = splitter-buffer;

		if(dataStart-1 < HANDLELENGTH){
			
			for(int shatter = dataStart+1; buflen>shatter; shatter += MAXMSG-1){
				PDU[0] = (dataStart-1);
				memcpy(PDU+1, buffer, dataStart-1);
				currlen = std::min(MAXMSG-1, buflen-shatter);
				memcpy(PDU+(dataStart), buffer+shatter, currlen);
				sendPDU(socket, PDU, currlen+dataStart, FLAG_M);
			}

		}
		else{
			printf("input handle is too long\n");
		}
	}
	else{
		printf("invalid M formatting\n");
	}
	
}

void recvFromServer(int serverSock){
	uint8_t dataBuffer[MAXBUF];
	recvPDU(serverSock, dataBuffer, MAXBUF);
	printf("->%s\n", dataBuffer+2);
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
	if(strlen(argv[3]) > HANDLELENGTH-1){
		printf("handle [%s] too long\n",argv[4]);
		exit(-1);
	}
	else if(argv[3][0]<65 || argv[3][0] >122){
			printf("handle must start with an alphabetical character\n");
			exit(-1);
		}
}

void sendHandshake(int serverSocket, char* handle){

	uint8_t buffer[MAXBUF];
	uint32_t hLen = strlen(handle);
	printf("prepping %s with length %d\n", handle, hLen);
	
	buffer[0] = hLen;
	memcpy(buffer+1, handle, hLen+1);
	sendPDU(serverSocket, buffer, hLen+1, FLAG_NEWCLIENT);

}