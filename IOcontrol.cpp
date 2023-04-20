#include "IOcontrol.hpp"


int sendPDU(int clientSocket, uint8_t* dataBuffer, int lengthOfData){

    int retval = 0;
    int fullLen = lengthOfData+LENGTHFIELD;
    uint8_t* PDU = (uint8_t*)sCalloc(fullLen,sizeof(uint8_t));

    *(uint16_t*)PDU = htons(fullLen);

    memcpy(PDU+LENGTHFIELD, dataBuffer, lengthOfData);

    retval = safeSend(clientSocket, PDU, fullLen, 0);

    free(PDU);

    return retval-LENGTHFIELD;
}

int recvPDU(int socketNumber, uint8_t* dataBuffer, int bufferSize){

    int dataLength = 0;
    safeRecv(socketNumber, dataBuffer, LENGTHFIELD, 0);
    dataLength = ntohs(*(uint16_t*)dataBuffer);

    if(dataLength-LENGTHFIELD > bufferSize){
        perror("packet too large\n");
        exit(-1);
    }

    return safeRecv(socketNumber, dataBuffer, dataLength, 0);

}