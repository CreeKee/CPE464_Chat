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
    printf("[%s]\n", handle);

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

        case 'c':
            compileCM(buffer+5, sendLen, buffer[3]-'0', FLAG_C);
            break;

		case 'b':
			compileB(buffer+3, sendLen);
			break;

        case 'l':
            compileL();
            break;

		default:
			printf("unkown command %c%c\n",buffer[0], buffer[1]);
			break;
	}
}

void Client::compileL(){
    uint8_t buffer[MAXBUF];
    sendPDU(serverSock, buffer, 0, FLAG_LREQUEST);
}

void Client::compileCM(uint8_t buffer[MAXBUF], int buflen, uint8_t dstCount, int flag){

	uint8_t PDU[MAXBUF] = {0};
    uint8_t* buf = buffer;
	int currlen;
	int dataStart = 0;
    int check = 0;

    if((dstCount == 1 && flag == FLAG_M) || (dstCount>=2 && dstCount<=9 && flag == FLAG_C)){

        PDU[0] = myhLen;
        memcpy(PDU+1, handle, myhLen);
        PDU[myhLen+1] = dstCount;

        dataStart += myhLen+2;

        for(int dest = 0; dest < dstCount && check >=0; dest++){
            dataStart += (check = appendHandle((PDU+dataStart), &buf))+1;
            buflen -= check;
        }

        if(check>=0){
            
            fragment(PDU, buf, buflen, dataStart, flag);

        }
    }
    else{
        printf("invalid destination client count %d\n", dstCount);
    }
}

void Client::fragment(uint8_t PDU[MAXBUF], uint8_t* buffer, int buflen, int dataStart, int flag){
    
    //TODO broken
    int currlen;
    for(int shatter = 0; buflen>shatter; shatter += MAXMSG-1){

        currlen = std::min(MAXMSG-1, buflen-shatter);
        memcpy(PDU+(dataStart), buffer+shatter, currlen);
        sendPDU(serverSock, PDU, currlen+dataStart, flag);

    }
}

void Client::compileB(uint8_t buffer[MAXBUF], int buflen){
	
    uint8_t PDU[MAXBUF] = {0};
    
    PDU[0] = myhLen;
    memcpy(PDU+1, handle, myhLen);

    fragment(PDU, buffer, buflen, myhLen+1, FLAG_B);
}

void Client::recvFromServer(int serverSock){
	uint8_t dataBuffer[MAXBUF];
	recvPDU(serverSock, dataBuffer, MAXBUF);
	printf("%d->%s\n", dataBuffer[2], dataBuffer+2);
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