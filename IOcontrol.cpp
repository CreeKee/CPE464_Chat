#include "IOcontrol.hpp"


int sendPDU(int clientSocket, uint8_t* dataBuffer){

    int retval = 0;
    

    //retval = safeSend(clientSocket, PDU, fullLen, 0);

    

    return retval-LENGTHFIELD;
}

void prependLength(uint8_t* dataBuffer, int lengthOfData){

    int fullLen = lengthOfData+LENGTHFIELD;
    uint8_t* PDU = (uint8_t*)sCalloc(fullLen,sizeof(uint8_t));
    if(fullLen > MAXBUF){
        perror("PDU too long after length appended\n");
        exit(-1);
    }
    *(uint16_t*)PDU = htons(fullLen);

    memcpy(PDU+LENGTHFIELD, dataBuffer, lengthOfData);

    free(PDU);

    return;
}

int recvPDU(int socketNumber, uint8_t* dataBuffer, int bufferSize){

    int dataLength = 0;

    //recieve incoming PDU length
    int retval = safeRecv(socketNumber, dataBuffer, LENGTHFIELD, MSG_WAITALL);

    //convert message length from network order to host order
    dataLength = ntohs(*(uint16_t*)dataBuffer)-LENGTHFIELD;

    //check for disconnection
    if(retval != 0){
        if(dataLength > bufferSize){
            perror("packet too large\n");
            exit(-1);
        }

        //recieve full message
        retval = safeRecv(socketNumber, dataBuffer, dataLength, MSG_WAITALL);
    }

    return retval;
}