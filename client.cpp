#include "client.hpp"

Client::Client(uint32_t socket, uint8_t myhandle[HANDLELENGTH]){

	//initialize client prarameters
	strcpy((char*)handle, (char*)myhandle);
	myhLen = strlen((char*)handle);
	serverSock = socket;

    //create poll
	setupPollSet();

	//add serverSocket, and stdin to listen for incoming and outgoing messages
	addToPollSet(serverSock);
	addToPollSet(STDIN_FILENO);

	//confirm connection and signal server that client is ready
    sendHandshake();
}

/*
clientAction recieves and proccessess an incoming packet from the 
server or reads from stdin and compiles then sends an outgoing packet
*/
void Client::clientAction(){

    int action;

	//poll options
    if((action = pollCall(-1)) != -1){

		switch(action){

			//read input from stdin, create packet, then send it
			case STDIN_FILENO:
				createMessage();
				break;

			//recieve and process packet from server
			default:
				recvFromServer(action); 
				break;
		}
    }

	//display terminal symbol
	if(print$) printf("$: ");
	fflush(stdout);
}

int32_t Client::getcomp(uint8_t** s){
	while(**s && isspace(**s)){
		*s += 1;
	}
	return **s ? **s : -1;
}

/*
createMessage reads from stdin, checks the formatting and selects 
the next action
*/
void Client::createMessage(){

	uint8_t buffer[MAXBUF] = {0};   //data buffer
	int sendLen = 0;        //amount of data to send
	uint8_t* s = buffer;
	int32_t comp;
	int32_t targets;
	
	//read input from stdin
	sendLen = readFromStdin(buffer);


	comp = getcomp(&s);
	

	//check that input starts with '%'
	if(comp == -1 || comp != '%'){
		printf("Invalid command format\n");
	} 

	//select command type and verify input formatting
    else {
		s++;
		if((comp = getcomp(&s))!= -1){
			s++;
			switch(tolower(buffer[1])){
				case 'm':
					if(getcomp(&s)!= -1) compileCM(s, sendLen, 1, FLAG_M);
					else printf("Invalid command format\n");
					break;

				case 'c':
					targets = getcomp(&s)-'0';
					if(targets>0 && getcomp(&s) != -1) compileCM(s, sendLen, targets, FLAG_C);
					else printf("Invalid command format\n");
					break;

				case 'b':
					if(getcomp(&s)!= -1) compileB(s, sendLen);
					else printf("Invalid command format\n");
					break;

				case 'l':
					//does not matter if any additional message is added on, just as long as the %L exists
					printf("made it this far");
					compileL();
					break;

				case 'e':
					//does not matter if any additional message is added on, just as long as the %E exists
					compileE();
					break;

				default:
					printf("Invalid command\n");
					break;
			}
		}
	}
}

/*
compileE sends a packet to the server with flag indicating a %E message
*/
void Client::compileE(){
	uint8_t buffer[MAXBUF] = {0};
	removeFromPollSet(STDIN_FILENO);
	sendPDU(serverSock, buffer, 0, FLAG_E);
	print$ = false;
}

/*
compileL sends a packet to the server with flag indicating a %L request
*/
void Client::compileL(){
    uint8_t buffer[MAXBUF] = {0};
    sendPDU(serverSock, buffer, 0, FLAG_LREQUEST);
}

/*
compileCM sends a packet to the server with flag indicating a %C or %M message
that packet contains the fully formatted message
*/
void Client::compileCM(uint8_t buffer[MAXBUF], int buflen, uint8_t dstCount, int flag){

	uint8_t PDU[MAXBUF] = {0};
    uint8_t* buf = buffer;
	int dataStart = 0;
    int check = 0;
	int bufRemaining = buflen;

	//confirm that destination counts are correct
    if((dstCount == 1 && flag == FLAG_M) || (dstCount>=2 && dstCount<=9 && flag == FLAG_C)){

		//assign handle length, handle, and destination count
        PDU[0] = myhLen;
        memcpy(PDU+1, handle, myhLen);
        PDU[myhLen+1] = dstCount;

        dataStart = myhLen+2;

		//gather destination clients and attatch them to the packet
        for(int dest = 0; dest < dstCount && check >=0; dest++){
            dataStart += (check = appendHandle((PDU+dataStart), &buf))+1;
            bufRemaining -= check;
        }

        if(check>=0){
            //send message
            fragment(PDU, buf, bufRemaining-1, dataStart, flag);

        }
		else{
			printf("Invalid command format. Invalid handles\n");
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

/*
compileB creates a broadcast packet and sends it to the server
*/
void Client::compileB(uint8_t buffer[MAXBUF], int buflen){
	
    uint8_t PDU[MAXBUF] = {0};
    
	//add client handle to the packet
    PDU[0] = myhLen;
    memcpy(PDU+1, handle, myhLen);

	//send packet
    fragment(PDU, buffer, buflen, myhLen+1, FLAG_B);
}

/*
recvFromServer recieves a packet from the server and chooses the next action
*/
void Client::recvFromServer(int serverSock){

	uint8_t PDU[MAXBUF] = {0};
	int flag;
	int messageLength;

	//recieve message
	if((messageLength = recvPDU(serverSock, PDU, MAXBUF))>0){

		//extract flag
		flag = PDU[FLAGOFFSET];

		//select next action
		if( flag<= PRINTACTIONS && flag >=0){
			(this->*flagActions[flag])(PDU+3);
		}
		else{
			printf("bad flag read\n");
		}

	}
	else{
		//server was abruptly terminated. Close and exit.
		printf("Server Terminated\n");
		removeFromPollSet(serverSock);
		exit(0);
	}

}

/*
sendHandshake sends a packet to the server with flag = 01 to indicate a new client
*/
void Client::sendHandshake(){

	uint8_t buffer[MAXBUF] = {0};
	
	buffer[0] = myhLen;
	memcpy(buffer+1, handle, myhLen+1);
	sendPDU(serverSock, buffer, myhLen+1, FLAG_NEWCLIENT);

}

/*
displayCM reads out the sender, and contents of an incoming %C or %M message and prints it to stdout
*/
void Client::displayCM(RECVACTION){

	uint8_t offset = displayHandle(PDU)+1;
	uint8_t clnts = PDU[offset++];

	for(int dsts = 0; dsts < clnts; dsts++){
		offset += PDU[offset]+1;
	}
	printf("%s\n",PDU+offset);
}

/*
displayHandle reads and displays the senders handle form a %B, %C, or %M packet
*/
uint8_t Client::displayHandle(uint8_t PDU[MAXBUF]){

	uint8_t length = PDU[0];
	
	printf("\n%.*s: ", length, PDU+1);
	return length;
}

uint32_t Client::readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	uint32_t inputLen = 0;        
	
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
