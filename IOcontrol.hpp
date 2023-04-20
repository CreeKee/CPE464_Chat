#include "includes.hpp"
#include "safeUtil.h"
#include "networks.h"

#define LENGTHFIELD 2

int sendPDU(int clientSocket, uint8_t* dataBuffer, int lengthOfData);
int recvPDU(int socketNumber, uint8_t* dataBuffer, int bufferSize);