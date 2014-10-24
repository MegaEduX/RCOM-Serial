/*
 * RCOM - Application Layer
 * Grupo XXX
 */

#include <stdlib.h>

#include "ApplicationLayer.h"

void alsetup(int fd, kApplicationState state) {
	applicationLayerInstance = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
	
	applicationLayerInstance->fileDescriptor = fd;
	applicationLayerInstance->state = state;
}

char * makeDataPacket(int sequenceNumber, char *data, int dataLen) {
	char *dataPacket = (char *) malloc(sizeof(char) * (4 + dataLen));
	
	dataPacket[0] = (char)(kApplicationPacketControlData);
	dataPacket[1] = (char)(sequenceNumber % 256);
	dataPacket[2] = (char)(dataLen / 256);
	dataPacket[3] = (char)(dataLen % 256);
	
	int i = 4, j = 0;
	
	for (; j < dataLen; i++, j++)
		dataPacket[i] = data[j];
	
	return dataPacket;
}

char * makeControlPacket(kApplicationPacketControl type, TLVParameter *tlvArray, int size) {
	char *controlPacket = (char *) malloc(sizeof(char) * (1 + size));
	
	controlPacket[0] = (char)type;
	
	int i = 1, j = 0;
	
	for (; j < size; j++) {
		controlPacket[i++] = tlvArray[j].type;
		controlPacket[i++] = tlvArray[j].length;
		
		int k = 0;
		
		for (; k < tlvArray[j].length; k++)
			controlPacket[i++] = tlvArray[j].value[k];
	}
	
	return controlPacket;
}