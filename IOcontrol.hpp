#include "includes.hpp"
#include "safeUtil.h"
#include "networks.h"

#define LENGTHFIELD 2
#define CHATLENGTH 3

void sendPDU(int clientSocket, uint8_t* dataBuffer, int flag);
int recvPDU(int socketNumber, uint8_t* dataBuffer, int bufferSize);
void prependLength(uint8_t* dataBuffer, int lengthOfData);
int addChatHeader(uint8_t* dataBuffer, int lengthOfData, int flag);