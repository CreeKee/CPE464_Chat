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

	sendLen = readFromStdin(buffer);


	if(buffer[0] != '%'){
		printf("Invalid command format\n");
	} //TODO comman format checking
	
	//TODO magicnums
    else switch(tolower(buffer[1])){
		case 'm':
			if(buffer[2]== ' ') compileCM(buffer+3, sendLen, 1, FLAG_M);
			else printf("Invalid command format\n");
			break;

        case 'c':
			if(buffer[4]== ' ') compileCM(buffer+5, sendLen, buffer[3]-'0', FLAG_C);
			else printf("Invalid command format\n");
            break;

		case 'b':
			if(buffer[2]== ' ') compileB(buffer+3, sendLen);
			else printf("Invalid command format\n");
			break;

        case 'l':
            if(sendLen == 3) compileL();
			else printf("Invalid command format\n");
            break;

		case 'e':
			if(sendLen == 3)compileE();
			else printf("Invalid command format\n");
			break;
		default:
			printf("Invalid command\n");
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
            bufRemaining -= check;
        }

        if(check>=0){
            
            fragment(PDU, buf, bufRemaining-1, dataStart, flag);

        }
		else{
			printf("Invalid command format. Count does not match number of handles\n");
		}
    }
    else{
        printf("Invalid destination client count: %d\n", dstCount);
    }
}

/*
fragment will take a buffer and send it to the server for processing.
if the data in the buffer exceeds the 199 byte limit, it is broken up
and sent in multiple fragments as seperate packets.
*/
void Client::fragment(uint8_t PDU[MAXBUF], uint8_t* buffer, int buflen, int dataStart, int flag){
    
	//new buffer is used to avoid adding multiple chat headers due to mutability
	uint8_t newBuf[MAXBUF];
    int currlen;

    for(int shatter = 0; buflen>shatter; shatter += MAXMSG-1){

		//copy current header
		memcpy(newBuf,PDU,dataStart);

		//get current data segment
        currlen = std::min(MAXMSG-1, buflen-shatter);

		//add data segment to buffer
        memcpy(newBuf+(dataStart), buffer+shatter, currlen);

		//send fragmented data
        sendPDU(serverSock, newBuf, currlen+dataStart, flag);

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

	uint8_t offset = displayHandle(PDU)+1;
	uint8_t clnts = PDU[offset++];

	for(int dsts = 0; dsts < clnts; dsts++){
		offset += PDU[offset++];
	}
	printf("%s\n",PDU+offset);
}

uint8_t Client::displayHandle(uint8_t PDU[MAXBUF]){

	uint8_t length = PDU[0];
	
	printf("\n%.*s: ", length, PDU+1);
	return length;
}