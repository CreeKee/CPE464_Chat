#include "includes.hpp"
#include "safeUtil.h"
#include "networks.h"

#define LENGTHFIELD 2
#define CHATLENGTH 3

void forwardPDU(int sock, uint8_t PDU[MAXBUF], int len);
void sendPDU(int clientSocket, uint8_t* dataBuffer, int datalength, int flag);
int recvPDU(int socketNumber, uint8_t* dataBuffer, int bufferSize);
void prependLength(uint8_t* dataBuffer, int lengthOfData);
int addChatHeader(uint8_t* dataBuffer, int lengthOfData, int flag);
void insertHandle(uint8_t* PDUstart, uint8_t* handleStart, uint8_t hLen);
int appendHandle(uint8_t* PDU, uint8_t** buffer);