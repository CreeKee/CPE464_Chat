#include "client.hpp"

Client::Client(int socket, uint8_t myhandle[HANDLELENGTH]){
	strcpy((char*)handle, (char*)myhandle);
	myhLen = strlen((char*)handle);
	serverSock = socket;

    //create poll
	setupPollSet();
	//add server socket to poll set to listen for new connections
	addToPollSet(serverSock);
	addToPollSet(STDIN_FILENO);

    sendHandshake();

}

void Client::clientAction(){
    int action;
    if((action = pollCall(-1)) != -1){

            switch(action){
                case STDIN_FILENO:
                    createMessage();
                    break;
                default:
                    recvFromServer(action);
                    
                    
                    break;
            }
        }
}

void Client::createMessage(){

	uint8_t buffer[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0; 

	sendLen = readFromStdin(buffer);

	printf("readback: %s <->%s\n",buffer, buffer+3);

	if(buffer[0] != '%'){
		printf("all commands must start with '%'\n");
	} else if(buffer[2] != ' '){
		printf("invalid command format");
	}
	
    else switch(tolower(buffer[1])){
		case 'm':
			compileCM(buffer+3, sendLen, 1, FLAG_M);
			break;

		case 'b':
			//compileB(buffer+3, sendLen);
			break;

		default:
			printf("unkown command %c%c\n",buffer[0], buffer[1]);
			break;
	}
}

void Client::insertHandle(uint8_t* PDUstart, uint8_t* handleStart, uint8_t hLen){
	PDUstart[0] = hLen;
	memcpy(PDUstart+1, handleStart, hLen);
}

int Client::appendHandle(uint8_t* PDU, uint8_t* buffer){

	uint8_t* splitter = (uint8_t*)strchr((char*)buffer, ' ');
	int offset = 0;

	if(splitter != NULL){
		offset = splitter-buffer+1;

		if(offset < HANDLELENGTH){
			insertHandle(PDU, buffer, offset-1);
			PDU[0] = offset-1;
			memcpy(PDU+1, buffer, offset-1);
			buffer+=offset;
		}
		else{
			offset = -2;
			printf("input handle is too long\n");
		}
	}
	else{
		offset = -1;
		printf("invalid message formatting\n");
	}

    return offset;

}

void Client::compileCM(uint8_t buffer[MAXBUF], int buflen, uint8_t dstCount, int flag){

	uint8_t PDU[MAXBUF] = {0};
	int currlen;
	int dataStart = 0;
    int check = 0;
    //TODO verify sender count

    PDU[0] = myhLen;
    memcpy(PDU+1, handle, myhLen);
    PDU[myhLen+1] = dstCount;

    for(int dest = 0; dest<dstCount && check >=0; dest++){
        dataStart += (check = appendHandle((PDU+dataStart),buffer));
        buflen -= check;
    }

    if(check>=0){
        
        fragment(PDU, buffer, buflen, dataStart, flag);

    }
	
}

void Client::fragment(uint8_t PDU[MAXBUF], uint8_t buffer[MAXBUF], int buflen, int dataStart, int flag){
    int currlen;
    for(int shatter = 0; buflen>shatter; shatter += MAXMSG-1){

        currlen = std::min(MAXMSG-1, buflen-shatter);
        memcpy(PDU+(dataStart), buffer+shatter, currlen);
        sendPDU(serverSock, PDU, currlen+dataStart, flag);

    }
}



/*
void Client::compileB(uint8_t buffer[MAXBUF], int buflen, int socket){
	insertHandle(PDU, buffer, dataStart);
}*/

void Client::recvFromServer(int serverSock){
	uint8_t dataBuffer[MAXBUF];
	recvPDU(serverSock, dataBuffer, MAXBUF);
	printf("->%s\n", dataBuffer+2);
}

void Client::sendToServer(int socketNum)
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

int Client::readFromStdin(uint8_t * buffer)
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

void Client::sendHandshake(){

	uint8_t buffer[MAXBUF];
	printf("prepping %s with length %d\n", handle, myhLen);
	
	buffer[0] = myhLen;
	memcpy(buffer+1, handle, myhLen+1);
	sendPDU(serverSock, buffer, myhLen+1, FLAG_NEWCLIENT);

}