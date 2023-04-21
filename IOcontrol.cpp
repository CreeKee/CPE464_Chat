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
    int retval = safeRecv(socketNumber, dataBuffer, LENGTHFIELD, MSG_WAITALL);
    dataLength = ntohs(*(uint16_t*)dataBuffer)-LENGTHFIELD;

    if(retval != 0){
        if(dataLength > bufferSize){
            perror("packet too large\n");
            exit(-1);
        }
        retval = safeRecv(socketNumber, dataBuffer, dataLength, MSG_WAITALL);
    }

    return retval

}