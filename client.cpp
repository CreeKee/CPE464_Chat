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
	if(print$) printf("$: ");
	fflush(stdout);
}

void Client::createMessage(){

	uint8_t buffer[MAXBUF] = {0};   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0; 

	sendLen = readFromStdin(buffer);


	if(buffer[0] != '%'){
		printf("all commands must start with '%'\n");
	} //TODO comman format checking
	
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

		case 'e':
			compileE();
			break;
		default:
			printf("unkown command %c%c\n",buffer[0], buffer[1]);
			break;
	}
}

void Client::compileE(){
	uint8_t buffer[MAXBUF] = {0};
	sendPDU(serverSock, buffer, 0, FLAG_E);
	print$ = false;
}

void Client::compileL(){
    uint8_t buffer[MAXBUF] = {0};
    sendPDU(serverSock, buffer, 0, FLAG_LREQUEST);
}

void Client::compileCM(uint8_t buffer[MAXBUF], int buflen, uint8_t dstCount, int flag){

	uint8_t PDU[MAXBUF] = {0};
    uint8_t* buf = buffer;
	int currlen;
	int dataStart = 0;
    int check = 0;
	int bufRemaining = buflen;

    if((dstCount == 1 && flag == FLAG_M) || (dstCount>=2 && dstCount<=9 && flag == FLAG_C)){

        PDU[0] = myhLen;
        memcpy(PDU+1, handle, myhLen);
        PDU[myhLen+1] = dstCount;

        dataStart = myhLen+2;

        for(int dest = 0; dest < dstCount && check >=0; dest++){
            dataStart += (check = appendHandle((PDU+dataStart), &buf))+1;
			printf("offset %d %d\n",buflen, check);
            bufRemaining -= check;
        }

        if(check>=0){
            
            fragment(PDU, buf-1, bufRemaining-1, dataStart, flag);

        }
    }
    else{
        printf("Invalid destination client count: %d\n", dstCount);
    }
}

void Client::fragment(uint8_t PDU[MAXBUF], uint8_t* buffer, int buflen, int dataStart, int flag){
    
    //TODO broken
    int currlen;
	printf("->%s %d %d\n",PDU, buflen, dataStart);
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

	uint8_t PDU[MAXBUF] = {0};
	int flag;
	int messageLength;

	if((messageLength = recvPDU(serverSock, PDU, MAXBUF))>0){

		flag = PDU[FLAGOFFSET];

		if( flag<= PRINTACTIONS && flag >=0){
			(this->*flagActions[flag])(PDU+3);
		}
		else{
			printf("bad flag read\n");
		}

	}
	else{
		printf("Server Terminated\n");
		removeFromPollSet(serverSock);
		exit(0);
	}

}

void Client::sendToServer(int socketNum)
{
	uint8_t sendBuf[MAXBUF] = {0};   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
	
	sendLen = readFromStdin(sendBuf);
	
	prependLength(sendBuf,sendLen);
	sent = safeSend(socketNum, sendBuf, sendLen+LENGTHFIELD, 0) - LENGTHFIELD;
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

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

	uint8_t buffer[MAXBUF] = {0};
	
	buffer[0] = myhLen;
	memcpy(buffer+1, handle, myhLen+1);
	sendPDU(serverSock, buffer, myhLen+1, FLAG_NEWCLIENT);

}

void Client::displayCM(RECVACTION){
	int offset = displayHandle(PDU)+1;

	for(int dsts = 0; dsts < PDU[offset]; dsts++){
		offset+=(PDU+(offset++))[0];
	}
	printf("%s\n",PDU+offset);
}


uint8_t Client::displayHandle(uint8_t PDU[MAXBUF]){
	uint8_t length = PDU[0];
	printf("\n%.*s: ", length, PDU+1);
	return length;
}