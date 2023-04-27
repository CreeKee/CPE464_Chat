#include "IOcontrol.hpp"


void forwardPDU(int sock, uint8_t PDU[MAXBUF], int len){
    if(safeSend(sock, PDU, len, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}

void sendPDU(int clientSocket, uint8_t* dataBuffer, int datalength, int flag){

    if (safeSend(clientSocket, dataBuffer, addChatHeader(dataBuffer, datalength, flag), 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}

void prependLength(uint8_t* dataBuffer, int lengthOfData){
    //TODO
    int fullLen = lengthOfData+LENGTHFIELD;
    uint8_t* PDU = (uint8_t*)sCalloc(fullLen,sizeof(uint8_t));

    if(fullLen > MAXBUF){
        perror("PDU too long after length appended\n");
        exit(-1);
    }
    *(uint16_t*)PDU = htons(fullLen);
    memcpy(PDU+LENGTHFIELD, dataBuffer, lengthOfData);
    memcpy(dataBuffer, PDU, fullLen);
    free(PDU);

    return;
}

int recvPDU(int socketNumber, uint8_t* dataBuffer, int bufferSize){

    int dataLength = 0;

    //recieve incoming PDU length
    int retval = safeRecv(socketNumber, dataBuffer, LENGTHFIELD, MSG_WAITALL);

    //check for disconnection
    if(retval != 0){

        //convert message length from network order to host order
        dataLength = ntohs(*(uint16_t*)dataBuffer)-LENGTHFIELD;
        printf("recieve packet with length field: %d\n", dataLength);
        
        if(dataLength > bufferSize){
            perror("packet too large\n");
            exit(-1);
        }

        //recieve full message
        retval = safeRecv(socketNumber, dataBuffer+LENGTHFIELD, dataLength, MSG_WAITALL);
        if(retval > 0){
            retval += LENGTHFIELD;
        }
    }


    return retval;
}

int addChatHeader(uint8_t* dataBuffer, int lengthOfData, int flag){
    int fullLen = lengthOfData+CHATLENGTH;
    uint8_t* PDU = (uint8_t*)sCalloc(fullLen,sizeof(uint8_t));

    if(fullLen > MAXBUF){
        perror("PDU too long after length appended\n");
        exit(-1);
    }
    *(uint16_t*)PDU = htons(fullLen);
    *(uint16_t*)(PDU+2) = flag;

    memcpy(PDU+CHATLENGTH, dataBuffer, lengthOfData);
    memcpy(dataBuffer, PDU, fullLen);
    printf("verifying %s\n",PDU+4);
    free(PDU);
    return fullLen;
}

void insertHandle(uint8_t* PDUstart, uint8_t* handleStart, uint8_t hLen){
	PDUstart[0] = hLen;
	memcpy(PDUstart+1, handleStart, hLen);
}

int appendHandle(uint8_t* PDU, uint8_t** buffer){

	uint8_t* splitter = (uint8_t*)strchr((char*)*buffer, ' ');
	int offset = 0;

	if(splitter != NULL){
		offset = splitter-*buffer;

		if(offset < HANDLELENGTH){
			PDU[0] = offset;
			memcpy(PDU+1, *buffer, offset);
			*buffer += offset+1;
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
