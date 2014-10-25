/*
 * RCOM - Link Layer
 * Grupo XXX
 */

#include <stdlib.h>
#include <string.h>

#include "LinkLayer.h"
 
void llsetup(char *port, int baudRate, unsigned int sequenceNumber, unsigned int timeout, unsigned int maxRetries) {
	linkLayerInstance = (LinkLayer *) malloc(sizeof(LinkLayer));
	
	//malloc()
	
	/*strcpy(linkLayerInstance->port, port);
	
	linkLayerInstance->baudRate = baudRate;
	linkLayerInstance->sequenceNumber = sequenceNumber;
	linkLayerInstance->timeout = timeout;
	linkLayerInstance->maxRetries = maxRetries;*/
}

void lltoggle() {
	linkLayerInstance->sequenceNumber = !(linkLayerInstance->sequenceNumber);
}
