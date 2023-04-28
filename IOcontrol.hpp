#include "includes.hpp"
#include "safeUtil.h"
#include "networks.h"

#define LENGTHFIELD 2
#define CHATLENGTH 3

void forwardPDU(uint32_t sock, uint8_t PDU[MAXBUF], uint32_t len);
void sendPDU(uint32_t clientSocket, uint8_t* dataBuffer, uint32_t datalength, uint8_t flag);
int32_t recvPDU(uint32_t socketNumber, uint8_t* dataBuffer, uint32_t bufferSize);
uint32_t addChatHeader(uint8_t* dataBuffer, uint32_t lengthOfData, uint8_t flag);
void insertHandle(uint8_t* PDUstart, uint8_t* handleStart, uint8_t hLen);
int32_t appendHandle(uint8_t* PDU, uint8_t** buffer);