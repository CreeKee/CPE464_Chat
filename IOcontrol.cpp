#include "IOcontrol.hpp"

/*
forwardPDU sends an unmodified packet to target socket
*/
void forwardPDU(uint32_t sock, uint8_t PDU[MAXBUF], uint32_t len){
    if(safeSend(sock, PDU, len, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}

/*
sendPDU attatches a chat header to a packet then sends it to target socket
*/
void sendPDU(uint32_t clientSocket, uint8_t* dataBuffer, uint32_t datalength, uint8_t flag){
    
    if (safeSend(clientSocket, dataBuffer, addChatHeader(dataBuffer, datalength, flag), 0) < 0)
    {
        perror("send call");
        exit(-1);
    }

}

/*
recvPDU recieves a fully formated PDU into the given buffer. First recieving 
the length field, then the full remaining packet.
*/
int32_t recvPDU(uint32_t socketNumber, uint8_t* dataBuffer, uint32_t bufferSize){

    uint32_t dataLength = 0;

    //recieve incoming PDU length
    int32_t retval = safeRecv(socketNumber, dataBuffer, LENGTHFIELD, MSG_WAITALL);

    //check for disconnection
    if(retval != 0){

        //convert message length from network order to host order
        dataLength = ntohs(*(uint16_t*)dataBuffer)-LENGTHFIELD;
        
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

/*
addChatHeader prepends a fully formated chat header to a given data buffer
*/
uint32_t addChatHeader(uint8_t* dataBuffer, uint32_t lengthOfData, uint8_t flag){

    uint32_t fullLen = lengthOfData+CHATLENGTH;
    uint8_t holldover[MAXBUF] = {0};

    if(fullLen < MAXBUF){
        *(uint16_t*)holldover = htons(fullLen);
        *(holldover+2) = flag;
        memcpy(holldover+CHATLENGTH, dataBuffer,lengthOfData);
        
        memcpy(dataBuffer, holldover, fullLen);
        
    }
    else{
        perror("PDU too long after chat header was added\n");
        exit(-1);
    }


    return fullLen;
}

/*
insertHandle takes a handle and its length and adds it to a given data buffer
*/
void insertHandle(uint8_t* PDUstart, uint8_t* handleStart, uint8_t hLen){
	PDUstart[0] = hLen;
	memcpy(PDUstart+1, handleStart, hLen);
}

/*
appendHandle extracts a handle from a buffer, calculates its length, and adds it
to the given PDU. It then increments the buffer past the processed handle.
*/
int32_t appendHandle(uint8_t* PDU, uint8_t** buffer){

    //locate end of handle
	uint8_t* splitter = (uint8_t*)strchr((char*)*buffer, ' ');
	int32_t offset = 0;

	if(splitter != NULL){

        //calculate length of handle
		offset = splitter-*buffer;

		if(offset < HANDLELENGTH){

            //insert handle and increment buffer
			PDU[0] = offset;
			memcpy(PDU+1, *buffer, offset);
			*buffer += offset+1;
		}
		else{
			offset = -2;
			printf("Invalid handle, handle longer than 100 characters %.*s\n", offset, *buffer);
		}
	}
	else{
		offset = -1;
		printf("invalid message formatting\n");
	}

    return offset;

}
