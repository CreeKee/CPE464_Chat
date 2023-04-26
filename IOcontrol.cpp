#include "IOcontrol.hpp"


void sendPDU(int clientSocket, uint8_t* dataBuffer, int flag){

    if (safeSend(clientSocket, dataBuffer, addChatHeader(dataBuffer, 0, flag), 0) < 0)
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

    //convert message length from network order to host order
    dataLength = ntohs(*(uint16_t*)dataBuffer)-LENGTHFIELD;
    printf("recieve packet with length field: %d\n", dataLength);

    //check for disconnection
    if(retval != 0){
        if(dataLength > bufferSize){
            perror("packet too large\n");
            exit(-1);
        }

        //recieve full message
        retval = safeRecv(socketNumber, dataBuffer+LENGTHFIELD, dataLength, MSG_WAITALL);
    }

    return retval+LENGTHFIELD;
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